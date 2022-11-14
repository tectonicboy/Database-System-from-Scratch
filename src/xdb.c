#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>

#define MAX_TABLES     64
#define ROW_ENTRY_SIZ  24
#define MAX_DATABASES  64
#define INITIAL_ROWS   10000000

#define NEXT_FREE_TBL     dbs[db_index]->tables[dbs[db_index]->active_tables]
#define THIS_TABLE        dbs[db_index]->tables[tbl_index]
  
/*  Parameterized macro to populate a buffer to be fed to Edit_Row().
 *  arg1: char* for the buffer to be fed to Edit_Row(). 
 *        The buffer this macro will be populating. 
 *  arg2: char* for a string of space-separated column names. \0-terminated.
 *  arg3: literal unsigned integer for number of columns in the table
 *  arg4: Auxilliary variable name (unsigned integer) accessible 
 *        from the same scope the macro is invoked in.
 */
#define CONSTRUCT_ROW_BUFFER(PTR, STR, COLS, AUX)     				    \
	memset((void*)(PTR), 0x0, ROW_ENTRY_SIZ * (COLS)); 			        \  
    for(size_t p = 0; p < (COLS); ++p){	  				                \
		while( (*(STR)) && (*(STR) != ' ') ){ 				            \
			++(AUX);						                            \
			++(STR);						                            \
	    } 								                                \
		strncpy(                                                        \
		        (PTR) + (p * ROW_ENTRY_SIZ)                             \
		       ,((STR) - (AUX))                                         \
		       ,(AUX)                                                   \
		       ); 	                                                    \
		++(STR);							                            \
		(AUX) = 0; 							                            \
	}

struct __attribute__ ((__packed__))  database {
	char db_name[64];
	size_t active_tables;	
	struct table* tables[64];
};

struct __attribute__ ((__packed__)) table {
	char tbl_name[64];
	size_t active_entries;
	uint8_t columns;
	uint32_t next_free_row;
	char* table_ptr;  /* memory address of the actual table */
	unsigned char table_full;
};

/*The global array of databases.*/
struct database* dbs[MAX_DATABASES];

/*How many databases are present in the system. */
size_t num_dbs = 0;

void print_red()    { printf("\033[1;31m");}

void print_yellow() { printf("\033[1;33m");}

void print_reset()  { printf("\033[0m"); }
                          
static void Create_Database(char* name){
	if(num_dbs == MAX_DATABASES){
	    printf("[ERR] Maximum number of databases already created.\n"); 
	    return; 
	}
	if(!name){ 
	    printf("[ERR] Empty database name address."); 
	    return;  
	}
	dbs[num_dbs] = (struct database*)malloc(sizeof(struct database));
	memset(dbs[num_dbs]->db_name, 0x0, 64);
	strcpy(dbs[num_dbs]->db_name, name);
	memset(dbs[num_dbs]->tables, 0x0, 64*sizeof(struct table*));
	dbs[num_dbs]->active_tables = 0;
	++num_dbs;
	printf("[OK] Created database [%s] successfully.\n", name);
	return;
}

//Allocate enough memory for INITIAL_ROWS rows. Keep track of number of entries.
static void Create_Table(
                         uint8_t  table_full
                        ,uint8_t  cols
                        ,uint32_t next_free_row
                        ,uint32_t db_index
                        ,char*    tbl_name
                        )
{
	if(dbs[db_index]->active_tables == MAX_TABLES){ 
	    printf("[ERR] Database already has max number of tables.\n"); 
	    return; 
	}
	if(!tbl_name){
	    printf("[ERR] Empty address of name of table to be added.\n"); 
	    return;
	}
	NEXT_FREE_TBL = malloc(sizeof(struct table));
	memset(NEXT_FREE_TBL->tbl_name, 0x0, 64);
	strcpy(
		    NEXT_FREE_TBL->tbl_name,
	       	tbl_name
	      );
    NEXT_FREE_TBL->table_ptr = malloc(INITIAL_ROWS * cols * ROW_ENTRY_SIZ);
	NEXT_FREE_TBL->columns = cols;
	NEXT_FREE_TBL->next_free_row = next_free_row;
	NEXT_FREE_TBL->table_full = table_full;
	++dbs[db_index]->active_tables;
	printf(
	        "[OK] Table [%s] added to database [%s] successfully.\n"
	       ,tbl_name
	       ,dbs[db_index]->db_name
	      );
}	

/* We know how much memory a row will contain by checking that table in
 * that database. That's why we only need an address, then
 * we can start reading chunks of ROW_ENTRY_SIZ bytes.
 */
static void Add_Row(size_t db_index, size_t tbl_index, char* contents_address){
	if(!contents_address){ 
	    printf("[ERR] Empty memory location of contents while adding a row.\n"); 
	    return; 
	}
	if(THIS_TABLE->table_full) { 
	    printf(  "[ERR] Unable to add entry to table [%s] in database [%s], "
	             "the table has no free entries.\n"
				,THIS_TABLE->tbl_name
				,dbs[db_index]->db_name
			  ); 
	    return; 
	}
        
	char* struct_mem = THIS_TABLE->table_ptr;

	/* Get struct_mem to the memory location of the desired row
	 * in the table and start populating that row.
	 */
	struct_mem +=   THIS_TABLE->next_free_row 
	              * THIS_TABLE->columns 
	              * ROW_ENTRY_SIZ;

	for(size_t i = 0; i < THIS_TABLE->columns; ++i){
            memcpy((void*)struct_mem, (void*)contents_address, ROW_ENTRY_SIZ);
            struct_mem += ROW_ENTRY_SIZ;
            contents_address += ROW_ENTRY_SIZ;
    }

	++THIS_TABLE->active_entries;

    printf(
            "[OK] Edited entry [%u] in table [%s] in database [%s]\n",
            THIS_TABLE->next_free_row,
            THIS_TABLE->tbl_name,
            dbs[db_index]->db_name
          );

	++THIS_TABLE->next_free_row;
	size_t free_row_i = THIS_TABLE->next_free_row;
	
	/* Calculate the next free row.
     * Two possible outcomes from this for-loop: 
	 * Either all entries were full, so free_row_i == INITIAL_ROWS
     * Or it found an entry beginning in \0, so free_row_i < INITIAL_ROWS 
     */
	for(char* next_row@ = struct_mem ; ; 
	    next_row@ += THIS_TABLE->columns * ROW_ENTRY_SIZ)
	{
		if( (*next_row@) && (free_row_i < INITIAL_ROWS) ) {
		    ++free_row_i; 
		    continue; 
		}
		else { break; }	
	}

	if( free_row_i < INITIAL_ROWS ){ THIS_TABLE->next_free_row = free_row_i; }
	else { THIS_TABLE->table_full = 1; }
}

static void Print_Table(size_t db_index, size_t tbl_index){
	print_red();
	printf(
	         "\n\n******************** PRINTING TABLE [%s] in database [%s]" 
	         "*********************\n\n"
		    ,THIS_TABLE->tbl_name
		    ,dbs[db_index]->db_name	
		  ); 
    print_reset();
	char* struct_mem = THIS_TABLE->table_ptr;
	//Memory size of each row including |, \n and \0 at the ends of entries.
	const size_t row_siz = (ROW_ENTRY_SIZ * THIS_TABLE->columns);
	size_t entry_siz, spaces;
	char* curr_entry;
	short odd_flag = 0;
	for(size_t i = 0; i < THIS_TABLE->active_entries; ++i){
		for(size_t j = 0; j < THIS_TABLE->columns; ++j){
			printf("|");
			curr_entry = struct_mem + (i * row_siz) + (j * ROW_ENTRY_SIZ);
                	entry_siz = strlen(curr_entry);
                	spaces = ROW_ENTRY_SIZ - entry_siz;
                	if(spaces % 2 == 1){spaces -= 1; odd_flag = 1;}
                	spaces /= 2;
                	for(size_t k = 0; k < spaces; ++k){ printf(" "); }
                	if(odd_flag){
                        	odd_flag = 0;
                        	++spaces;
               		}
			printf("%s", curr_entry);
			for(size_t k = 0; k < spaces; ++k){ printf(" "); }	
		}
		printf("|\n");
	}
	print_red();
	printf("\n\n******************************** END OF TABLE"
	       "*********************************\n\n");
	print_reset();
}	

void Delete_Row(size_t db_index, size_t tbl_index, size_t row_index){
	memset(
		    THIS_TABLE->table_ptr
		        + (row_index * THIS_TABLE->columns * ROW_ENTRY_SIZ)
		    ,0x0
		    ,THIS_TABLE->columns * ROW_ENTRY_SIZ
	      );
	if(THIS_TABLE->table_full) {
	    THIS_TABLE->next_free_row = row_index;  
		THIS_TABLE->table_full = 0;
	}
	else if(row_index < THIS_TABLE->next_free_row){ 
	    THIS_TABLE->next_free_row = row_index; 
	} 
	printf(
	        "[OK] Erased row [%lu] of table [%s] in database [%s]\n"
	        ,row_index
	        ,THIS_TABLE->tbl_name
	        ,dbs[db_index]->db_name
	      );
}


void Save_System(){
	print_yellow(); printf("\n\n\n********************** INITIATING SYSTEM SAVING *******************************\n\n\n"); print_reset();
	FILE* savefile;
	size_t bytes_written = 0, tables = 0, entries = 0, cols = 0;
	if(!(savefile = fopen("xdb_saved.dat", "w+"))){ printf("[ERR] Could not open save file.\n"); }
	else{printf("[OK] Opened save file\n");}
	if(!(bytes_written = fwrite(&num_dbs, 1, 8, savefile)))
		{printf("[ERR] Could not write # of databases to savefile.\n");}
	else{printf("[OK] Wrote # of databases to savefile.\n");}
	for(size_t a = 0; dbs[a]; ++a){
		if(!(bytes_written = fwrite(&dbs[a]->db_name, 1, 64, savefile)))
			{ print_red(); printf("[ERR] Could not write database name [%s] to savefile.\n", dbs[a]->db_name); print_reset(); }
		else{printf("[OK] Wrote to savefile %lu bytes of database name for [%s]\n", bytes_written, dbs[a]->db_name);}

		tables = dbs[a]->active_tables;
		if(!(bytes_written = fwrite(&dbs[a]->active_tables, 1, 8, savefile)))
			{ print_red(); printf("[ERR] Could not write # of tables in database [%s] to savefile.\n", dbs[a]->db_name); print_reset(); }
		else{printf("[OK] Wrote to savefile # of tables in database [%s].\n", dbs[a]->db_name);}
		for(size_t b = 0; b < tables; ++b){
			if(!(bytes_written = fwrite(dbs[a]->tables[b]->tbl_name, 1, 64, savefile)))
				{ print_red(); printf("[ERR] Could not write table name [%s] in database [%s] to savefile.\n"
						      , dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); print_reset(); }
			else{ printf("[OK] Wrote to savefile %lu bytes of table name [%s] in database [%s].\n"
				     , bytes_written, dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); }

                        if(!(bytes_written = fwrite(&dbs[a]->tables[b]->next_free_row, 1, 4, savefile)))
                                { print_red(); printf("[ERR] Could not write next free row of table [%s] in database [%s] to savefile.\n"
						      , dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Wrote to savefile next free row of table [%s] in database [%s].\n"
				     , dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); }

                        if(!(bytes_written = fwrite(&dbs[a]->tables[b]->table_full, 1, 1, savefile)))
                                { print_red(); printf("[ERR] Could not write table full flag of table [%s] in database [%s] to savefile.\n",
                                                dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Wrote to savefile the table full flag of table [%s] in database [%s].\n",
                                        	dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); }


			if(!(bytes_written = fwrite(&dbs[a]->tables[b]->active_entries, 1, 8, savefile)))
                                { print_red(); printf("[ERR] Could not write # of entries of table [%s] in database [%s]to savefile.\n",
					       	dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Wrote to savefile %lu bytes for # of entries in table [%s] in database [%s].\n", 
					bytes_written, dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); }

			if(!(bytes_written = fwrite(&dbs[a]->tables[b]->columns, 1, 8, savefile)))
                                { print_red(); printf("[ERR] Could not write # of columns of table [%s] in database [%s] to savefile.\n",
					       	dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Wrote to savefile %lu bytes for # of columns of table [%s] in database [%s].\n",
				       	bytes_written, dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); }

			cols = dbs[a]->tables[b]->columns;
			if( !(entries = dbs[a]->tables[b]->active_entries) ){continue;}
			
			if( !(bytes_written = fwrite(
						      dbs[a]->tables[b]->table_ptr, 
						      1,
						      (24 * cols * entries),
						      savefile
						    )))
			{
				print_red(); printf("[ERR] Could not write contents of rows of table [%s] in database [%s] to savefile.\n", 
						dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); print_reset();
			}
			else { printf("[OK] Wrote to savefile %lu bytes of row contents of table [%s] in database [%s].\n", 
					bytes_written, dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); }
		}
	}	
	fclose(savefile);
	print_yellow(); printf("\n\n\n********************** SYSTEM SAVING SUCCESS ******************************\n\n\n"); print_reset();
}



void Load_System(){
	print_yellow(); printf("\n\n\n********************** INITIATING SYSTEM LOADING *******************************\n\n\n"); print_reset();
        FILE* savefile;
	char buffer[128];
	unsigned char table_full = 0;
	uint32_t next_free_row = 0;
	memset(buffer, 0x0, 128);
        size_t bytes_read = 0, databases = 0, tables = 0, rows = 0, cols = 0;
        if(!(savefile = fopen("xdb_saved.dat", "r"))){ printf("[ERR] Could not open save file.\n"); }
	else{printf("[OK] Opened save file.\n");}
	if(!(bytes_read = fread(&databases, 1, 8, savefile)))
		{ print_red(); printf("[ERR] Could not read # of databases from savefile.\n"); print_reset();}
	else{printf("[OK] Read # of databases from savefile: %lu\n", databases);}
        for(size_t a = 0; a < databases; ++a){
		if(!(bytes_read = fread(buffer, 1, 64, savefile)))
                        { print_red(); printf("[ERR] Could not read database name from savefile.\n"); print_reset(); }
                else{
			printf("[OK] Read from savefile %lu bytes of database name: [%s] --> Creating Database.\n", bytes_read, buffer);
			Create_Database(buffer);
		}
		memset(buffer, 0x0, 64);
		if(!(bytes_read = fread(&tables, 1, 8, savefile)))
			{ print_red(); printf("[ERR] Could not read # of active tables in database from savefile.\n"); print_reset(); }
		else{printf("[OK] Read # of tables in database [%s] from savefile: %lu tables.\n", dbs[a]->db_name, tables);}
		
                for(size_t b = 0; b < tables; ++b){
                        if(!(bytes_read = fread(buffer, 1, 64, savefile)))
                                { print_red(); printf("[ERR] Could not read table name of table [%s] in database [%s] from savefile.\n"
						     , buffer, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Read table name [%s] in database [%s] from savefile.\n", buffer, dbs[a]->db_name); }

                        if(!(bytes_read = fread(&next_free_row, 1, 4, savefile)))
                                { print_red(); printf("[ERR] Could not read next free row index of table [%s] in database [%s] from savefile.\n"
						      , buffer, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Read next free row index of table [%s] in database [%s] from savefile.\n"
			 	     , buffer, dbs[a]->db_name); }

                        if(!(bytes_read = fread(&table_full, 1, 1, savefile)))
                                { print_red(); printf("[ERR] Could not read the table_full flag of table [%s] in database [%s] from savefile.\n"
						      , buffer, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Read table_full flag of table [%s] in database [%s] from savefile.\n"
				     , buffer, dbs[a]->db_name); }


                        if(!(bytes_read = fread(buffer + 64, 1, 8, savefile)))
                                { print_red(); printf("[ERR] Could not read # of entries in table [%s] in database [%s] from savefile.\n"
						      , buffer, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Read from savefile # of entries in table [%s] in database [%s]: %lu entries.\n"
					              , buffer, dbs[a]->db_name, (size_t)(*(buffer + 64))); }

                        if(!(bytes_read = fread(buffer + 72, 1, 8, savefile)))
                                { print_red(); printf("[ERR] Could not read # of columns in table [%s] in database [%s] from savefile.\n"
						      , buffer, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Read from savefile # of columns in table [%s] in database [%s]: %lu columns.\n"
				     , buffer, dbs[a]->db_name, (size_t)(*(buffer + 72))); }
			
			cols = (size_t)(*(buffer + 72));
			rows = (size_t)(*(buffer + 64));

			Create_Table(table_full, next_free_row, cols, buffer, a);
			dbs[a]->tables[b]->active_entries = rows;
                    
			if(!rows) { memset(buffer, 0x0, 128); continue; }
			
                        if( !(bytes_read = fread(
                                                 (dbs[a]->tables[b]->table_ptr),
                                                 1,
                                                 (24 * cols * rows),
                                                 savefile
                                                )
			     )
			  )
                        {
                                print_red(); printf("[ERR] Could not read data of table [%s] in database [%s] from savefile.\n", 
						dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); print_reset();
                        }
                        else { 
				printf("[OK] Read from savefile %lu bytes of data for table [%s] in database [%s].\n", 
					bytes_read, dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); 
			}
			memset(buffer, 0x0, 128);
                }
        }
        fclose(savefile);
        print_yellow(); printf("\n\n\n*********************** SYSTEM LOADING SUCCESS ******************************\n\n\n"); print_reset();

}

void Process_XSI_Command(char* cmd, char* out_buf){
	char* tbl_name;
	uint8_t  aux   = 0;
        uint16_t pos   = 0;
        uint64_t flags = 0;
	char *row_string = malloc(256);
	char *row_buffer = malloc(256);
	char *str_old = row_string, *buf_old = row_buffer;
	memset(row_string, 0x0, 256);
	memset(row_buffer, 0x0, 256);

/* ***************** ADD DATABASE COMMAND BEGINS ******************************/


        /* Command to add a database. 
         * DB name must be 0-terminated, and at most 64 chars (including \0).
         * Example: add_db-Veterinarian\0
         */
        if(!strncmp(cmd, "add_db", 6) && (strlen(cmd) > 8)){ Create_Database(cmd + 7); return; }

/* ***************** ADD DATABASE COMMAND ENDS ********************************/


/* ***************** ADD TABLE COMMAND BEGINS ********************************/
        
	else if(!strncmp(cmd, "add_tbl", 7)){
                pos = 8;
                tbl_name = cmd + 8;
                while(*(cmd + pos) != '-'){ ++pos; }
                if(strncmp(cmd + pos, "-indb-", 6)){ 
			print_red(); printf("[ERR] Invalid command to add a table. Missing '-indb-'\n"); print_reset(); return;}
                else{
                        pos += 6;

                        /* Use the second byte of the 64-bit FLAGS to store the value of the
                         * inner-loop control variable that would have otherwise been lost.
			 * And for other similar temporary/auxilliary values.
                         */
                        for(uint8_t j = 0; j < num_dbs; ++j){
                                flags &= ~(((uint64_t)1) << 63);
                                for(uint8_t i = 0; ; ++i){
                                        if(*(cmd + pos + i) != *(dbs[j]->db_name + i)){
                                                flags |= (((uint64_t)1) << 63);
						*( (char*)&flags + 1 ) = i;
                                                break;
                                        }
                                        *( (char*)&flags + 1 ) = i;
                                }
                                if(
                                   ((flags & (((uint64_t)1) << 63)))
                                   &&
                                   (!(*(dbs[j]->db_name + (uint8_t)( *( (char*)&flags + 1 ) ) )))
				   &&
				   ( *(cmd + pos + (uint8_t)(*( (char*)&flags + 1 ))) == '-' )
                                  )
                                {
                                        pos += (uint8_t)(*( (char*)&flags + 1 )) + 1;
					*( (char*)&flags + 1 ) = (uint8_t)((*(cmd + pos)) - 48);
                                         
					/* Save j here so we can postpone the Create_Table call
					 * until we've made sure all the column names are valid.
					 */
					*( (char*)&flags + 2 ) = j;
                                        break;
                                }
                        }
			/* At the start of the column names, delimited with - */
			pos += 2;
			*((char*)&flags + 4) = (uint8_t)1;
			for(uint8_t i = 0; i < (uint8_t)(*( (char*)&flags + 1)); ++i){
				while(
				      *(cmd + pos) != '-'
				      && 
				      (uint8_t)( *( (char*)&flags + 3 ) ) < ROW_ENTRY_SIZ
				     )
				{
					++pos;
					*((char*)&flags + 3) += (uint8_t)1;
				}

				/*  Now, pos is at the byte immediately AFTER the row entry, 
				 *  and the 4th byte of flags holds how long that entry was.
				 *  Accumulate total size of all row entries in the 5th byte
				 *  of flags, so that we know where to write in row_string.
				 */
				if(
				   *((char*)&flags + 3) == (uint8_t)ROW_ENTRY_SIZ
				   &&
				   *(cmd + pos) != '-' 
				  )
				{
					print_red(); printf("[ERR] Invalid cmd to add a table. Column name too long.\n"); print_reset(); return;
				}
				
				/* Add the column name to the row_string to be fed to the memory construction macro */
				uint8_t pad = 1;
				
				strncpy(
					 row_string + ( (uint8_t)(*((char*)&flags + 4)) ) - 1 + (i*pad)
					,cmd + pos  - ( (uint8_t)(*((char*)&flags + 3)) )
					,(uint8_t)(*((char*)&flags + 3))
				       );	
				*((char*)&flags + 4) += (uint8_t)*((char*)&flags + 3);

				/* Add the required empty space between entries in said string */
				if(i < ((uint8_t)(*( (char*)&flags + 1))) - 1){
					row_string[( (uint8_t)(*((char*)&flags + 4)) ) - 1 + (i*pad)] = ' ';
					/* *((char*)&flags + 3) += (uint8_t)1; */
				}

				/* Reset count of current column name's length */
				*((char*)&flags + 3) = (uint8_t)0;

				/* Move past the dash and at the start of the next column name */
				++pos;
			}

			Create_Table(
           			      0
                                     ,0
                                     ,(uint8_t)(*( (char*)&flags + 1))
                                     ,tbl_name
                                     ,(uint8_t)(*( (char*)&flags + 2))
                                    );
			uint8_t columns = *( (char*)&flags + 1);
		        CONSTRUCT_ROW_BUFFER(row_buffer, row_string, columns, aux);

        		Add_Row(
				 (uint8_t)(*( (char*)&flags + 2))
				,dbs[(uint8_t)(*( (char*)&flags + 2))]->active_tables - 1
				,row_buffer
			       );
			row_string = str_old;
			row_buffer = buf_old;
			free(row_string);
			free(row_buffer);
                }
        }

/* ***************** ADD TABLE COMMAND ENDS ***********************************************************************************************/

/******************* ADD ROW COMMAND BEGINS **********************************************************************************************/
	
	else if(!strncmp(cmd, "add_row", 7)){
		pos = 7;
		if(strncmp(cmd + pos, "-indb-", 6))
			{print_red(); printf("[ERR] Invalid command to add a row. -indb- part missing.\n"); print_reset(); return;}
		pos += 6;
                flags= 0;
	   		/* Use the second byte of the 64-bit FLAGS to store the value of the
                         * inner-loop control variable that would have otherwise been lost.
                         * And for other similar temporary/auxilliary values.
                         */
                        for(uint8_t j = 0; j < num_dbs; ++j){
                                flags &= ~(((uint64_t)1) << 63);
                                for(uint8_t i = 0; ; ++i){
                                        if(*(cmd + pos + i) != *(dbs[j]->db_name + i)){
                                                flags |= (((uint64_t)1) << 63);
                                                *( (char*)&flags + 1 ) = i;
                                                break;
                                        }
                                        *( (char*)&flags + 1 ) = i;
                                }
                                if(
                                   ((flags & (((uint64_t)1) << 63)))
                                   &&
                                   (!(*(dbs[j]->db_name + (uint8_t)( *( (char*)&flags + 1 ) ) )))
				                   &&
                                   ( *(cmd + pos + (uint8_t)(*( (char*)&flags + 1 ))) == '-' )
                                  )
                                {
                                        pos += (uint8_t)(*( (char*)&flags + 1 )) ;
                                        *( (char*)&flags + 2 ) = j;
                                        break;
                                }
		        }
			
		if(strncmp(cmd + pos, "-intbl-", 7))
                        {print_red(); printf("[ERR] Invalid command to add a row. -intbl- part missing.\n"); print_reset(); return;}
		pos += 7;
		        for(uint8_t j = 0; j < dbs[(uint8_t)( *((char*)&flags + 2) )]->active_tables; ++j){
                                flags &= ~(((uint64_t)1) << 63);
                                for(uint8_t i = 0; ; ++i){
                                        if(*(cmd + pos + i) != *(dbs[(uint8_t)( *((char*)&flags + 2 ) )]->tables[j]->tbl_name + i)){
                                                flags |= (((uint64_t)1) << 63);
                                                *( (char*)&flags + 3 ) = i;
                                                break;
                                        }
                                        *( (char*)&flags + 3 ) = i;
                                }
                                if(
                                   ((flags & (((uint64_t)1) << 63)))
                                   &&
                                   (!(*(dbs[(uint8_t)( *((char*)&flags + 2 ) )]->tables[j]->tbl_name 
					+ (uint8_t)( *( (char*)&flags + 3 ) ) )))
                                   &&
                                   ( *(cmd + pos + (uint8_t)(*( (char*)&flags + 3 ))) == '-' )
				  )
                                {
                                        pos += (uint8_t)(*( (char*)&flags + 3 )) + 1;
					*( (char*)&flags + 1 ) = dbs[(uint8_t)( *((char*)&flags + 2 ) )]->tables[j]->columns;
                                        *( (char*)&flags + 4 ) = j;
                                        break;
                                }
                        }

		/*FLAGS +0  first bit is used for control
		 *	+1  number of columns in the table
		 *	+2  database index
		 *	+3  table name length
		 *	+4  table index
		 */

		/* At beginning of row names now */
		 *((char*)&flags + 6) = (uint8_t)1;
                        for(uint8_t i = 0; i < (uint8_t)(*( (char*)&flags + 1)); ++i){
                                while(
                                      *(cmd + pos) != '-'
                                      &&
                                      (uint8_t)( *( (char*)&flags + 5 ) ) < ROW_ENTRY_SIZ
                                     )
                                {
                                        ++pos;
                                        *((char*)&flags + 5) += (uint8_t)1;
                                }

                                /*  Now, pos is at the byte immediately AFTER the row entry, 
                                 *  and the 6th byte of flags holds how long that entry was.
                                 *  Accumulate total size of all row entries in the 7th byte
                                 *  of flags, so that we know where to write in row_string.
                                 */
                                if(
                                   *((char*)&flags + 5) == (uint8_t)ROW_ENTRY_SIZ
                                   &&
                                   *(cmd + pos) != '-'
                                  )
                                {
                                        print_red(); printf("[ERR] Invalid cmd to add a row. Row entry too long.\n"); print_reset(); return;
                                }

                                /* Add the column name to the row_string to be fed to the memory construction macro */
                                uint8_t pad = 1;

                                strncpy(
                                         row_string + ( (uint8_t)(*((char*)&flags + 6)) ) - 1 + (i*pad)
                                        ,cmd + pos  - ( (uint8_t)(*((char*)&flags + 5)) )
                                        ,(uint8_t)(*((char*)&flags + 5))
                                       );
                                *((char*)&flags + 6) += (uint8_t)*((char*)&flags + 5);

                                /* Add the required empty space between entries in said string */
                                if(i < ((uint8_t)(*( (char*)&flags + 1))) - 1){
                                        row_string[( (uint8_t)(*((char*)&flags + 6)) ) - 1 + (i*pad)] = ' ';
                                        /* *((char*)&flags + 3) += (uint8_t)1; */
                                }

                                /* Reset count of current column name's length */
                                *((char*)&flags + 5) = (uint8_t)0;

                                /* Move past the dash and at the start of the next column name */
                                ++pos;
                        }
			uint8_t columns = *( (char*)&flags + 1);
                        CONSTRUCT_ROW_BUFFER(row_buffer, row_string, columns, aux);

                        Add_Row(
                                 (uint8_t)(*( (char*)&flags + 2))
                                ,(uint8_t)(*( (char*)&flags + 4))
                                ,row_buffer
                               );
                        row_string = str_old;
                        row_buffer = buf_old;
                        free(row_string);
                        free(row_buffer);
	}

/* ***************** ADD ROW COMMAND ENDS ***********************************************************************************************/


}

int main(){
	memset(dbs, 0x0, 64*sizeof(struct database*));
/*	
	Load_System();
	Print_Table(0, 0);
	Print_Table(0, 1);
	Save_System();
	Load_System();
	Print_Table(0, 0);
	Print_Table(0, 1);
*/

 
	Create_Database("Veterinarian");
	Create_Table(0, 0, 4, "Pets", 0);

	char col_buffer[10 * ROW_ENTRY_SIZ];
	memset(col_buffer, 0x0, 10 * ROW_ENTRY_SIZ);
	size_t entry_siz_aux = 0;
	
        //  Usually this row string will come from a system source such
        //  as a web server on the databases' socket buffer (w/o entry #).
        //  Its bytes will be printable and a Line Feed will divide each entry.
        //  A null byte will be guaranteed to terminate the string.
	//  Each thread will have exactly one input string row buffer.
        //  For this example we just initialize a bunch of strings to test the macro.
	char *row0 = malloc(128), *row1 = malloc(128), *row2 = malloc(128), *row3 = malloc(128)
	    ,*row4 = malloc(128), *row5 = malloc(128), *row6 = malloc(128);	
	strcpy(row0, "1 Name Owner Condition\0");
	strcpy(row1, "2 Sarah Natalia Tubercolosis\0");
	strcpy(row2, "3 Usya NankaKalpazanka Lung_Disease\0");
	strcpy(row3, "4 Masha Kevin Code_Problems\0");
	strcpy(row4, "5 Miki Mimulya Happy\0");
	strcpy(row5, "6 Kiki Mimulya Happy\0");
	strcpy(row6, "7 Azor_Giraffe Baba_Marina Very_Happy\0");
		
	

        CONSTRUCT_ROW_BUFFER(col_buffer, row0, 4, entry_siz_aux);     
	Add_Row(0, 0, col_buffer);
	
	CONSTRUCT_ROW_BUFFER(col_buffer, row1, 4, entry_siz_aux);
	Add_Row(0, 0, col_buffer);

	CONSTRUCT_ROW_BUFFER(col_buffer, row2, 4, entry_siz_aux);
	Add_Row(0, 0, col_buffer);

	CONSTRUCT_ROW_BUFFER(col_buffer, row3, 4, entry_siz_aux);
	Add_Row(0, 0, col_buffer);

	CONSTRUCT_ROW_BUFFER(col_buffer, row4, 4, entry_siz_aux);
	Add_Row(0, 0, col_buffer);

	CONSTRUCT_ROW_BUFFER(col_buffer, row5, 4, entry_siz_aux);
	Add_Row(0, 0, col_buffer);

	CONSTRUCT_ROW_BUFFER(col_buffer, row6, 4, entry_siz_aux);
	Add_Row(0, 0, col_buffer);

	Print_Table(0, 0);

	Create_Table(0, 0, 4, "Doctors", 0);

	Save_System();

	char cmd[512];
	memset(cmd, 0x0, 512);
	/* The names of things that already exist in the system (database in the following example)
	 * dont have a \0 at the end. The things that don't exist (below table name) does have a \0
	 * at the end so that we can easily set a pointer to it and say "here this is the string".
	 * As for the row string, it gets copied to a zeroed-out buffer, so that the requirement
	 * of the row memory construction macro for null termination is met. - at the end is needed
	 * cuz the individual entries loop searches for a that char to end the entry, or when an
	 * entry has reached the allowed size for row entries - the ROW_ENTRY_SIZ macro. 
	 */
	memcpy((void*)cmd, "add_tbl-Offices\0-indb-Veterinarian-6-idk1-idk2-idk3-idk4-idk5-idk6-", 68);
	Process_XSI_Command(cmd, col_buffer);
	memset(cmd, 0x0, 512);
	/* So in this example neither the database name nor the table name will have \0 cuz they 
	 * will be checked against the names of already existing tables and databases.
	 */
	memcpy((void*)cmd, "add_row-indb-Veterinarian-intbl-Offices-wowzaa1-wowzaa2-wowzaa3-wowzaa4-wowzaa5-wowzaa6-", 89);
	Process_XSI_Command(cmd, col_buffer);
	Print_Table(0, 2);
	return 0;

}































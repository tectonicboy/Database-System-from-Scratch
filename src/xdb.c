#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>

#define MAX_TABLES     64
#define ROW_ENTRY_SIZ  24
#define MAX_DATABASES  64
#define INITIAL_ROWS   10000000

#define THIS_TABLE dbs[db_index]->tables[tbl_index]
  
/*  Parameterized macro to populate a buffer to be fed to Edit_Row().
 *  arg1: char* for buffer to be fed to Edit_Row(). 
 *  arg2: number of columns in table in which row is to be added.
 *  arg3: char* for \n-separated string containing each entry's string. \0-terminated.
 *  arg4: Auxilliary variable to hold the count of each entry's size.
 */
#define CONSTRUCT_ROW_BUFFER(PTR, STR, COLS, AUX)     				\
	memset((void*)(PTR), 0x0, ROW_ENTRY_SIZ * (COLS)); 			\
        for(size_t p = 0; p < (COLS); ++p){	  				\
		while( (*(STR)) && (*(STR) != ' ') ){ 				\
			++(AUX);						\
			++(STR);						\
	       	} 								\
		strncpy((PTR) + (p * ROW_ENTRY_SIZ), ((STR) - (AUX)), (AUX)); 	\
		++(STR);							\
		(AUX) = 0; 							\
	}									\

struct __attribute__ ((__packed__))  database {
	char db_name[64];
	size_t active_tables;	
	struct table* tables[64];
};

struct __attribute__ ((__packed__)) table {
	char tbl_name[64];
	size_t active_entries;
	size_t columns;
	size_t next_free_row;
	char* table_ptr; /* memory address of the actual table */
	unsigned char table_full;
};

//The global array of databases.
struct database* dbs[MAX_DATABASES];

//How many databases are present in the system.
size_t num_dbs = 0;

void print_red()    { printf("\033[1;31m");}

void print_yellow() { printf("\033[1;33m");}

void print_reset()  { printf("\033[0m"); }

static void Create_Database(char* name){
	if(num_dbs == MAX_DATABASES){ printf("[ERR] Maximum number of databases already created.\n"); return; }
	if(!name){ printf("[ERR] Empty database name address."); return;  }
	dbs[num_dbs] = (struct database*)malloc(sizeof(struct database));
	memset(dbs[num_dbs]->db_name, 0x0, 64);
	strcpy(dbs[num_dbs]->db_name, name);
	memset(dbs[num_dbs]->tables, 0x0, 64*sizeof(struct table*));
	dbs[num_dbs]->active_tables = 0;
	++num_dbs;
	printf("[OK] Created database [%s] successfully.\n", name);
}

//Initially allocate enough memory for INITIAL_ROWS rows. Keep track of number of entries.
static void Create_Table(unsigned char table_full, size_t next_free_row, size_t cols, char* tbl_name, size_t db_index){
	if(dbs[db_index]->active_tables == MAX_TABLES){ printf("[ERR] Database already has max number of tables.\n"); return; }
	if(!tbl_name){printf("[ERR] Empty address of table name to be added to the database.\n"); return;}
	dbs[db_index]->tables[dbs[db_index]->active_tables] = malloc(sizeof(struct table));
	memset(dbs[db_index]->tables[dbs[db_index]->active_tables]->tbl_name, 0x0, 64);
	strcpy(
		dbs[db_index]->tables[dbs[db_index]->active_tables]->tbl_name,
	       	tbl_name
	      );
        dbs[db_index]->tables[dbs[db_index]->active_tables]->table_ptr = malloc(INITIAL_ROWS * cols * ROW_ENTRY_SIZ);
	dbs[db_index]->tables[dbs[db_index]->active_tables]->columns = cols;
	dbs[db_index]->tables[dbs[db_index]->active_tables]->next_free_row = next_free_row;
	dbs[db_index]->tables[dbs[db_index]->active_tables]->table_full = table_full;
	++dbs[db_index]->active_tables;
	printf("[OK] Table [%s] added to database [%s] successfully.\n", tbl_name, dbs[db_index]->db_name);
}	

//We know how much memory a row will contain by checking that table in that database.
//That's why we only need an address, then we can start reading chunks of ROW_ENTRY_SIZ bytes.
static void Add_Row(size_t db_index, size_t tbl_index, char* contents_address){
	if(!contents_address){ printf("[ERR] Empty memory location of contents while adding a row.\n"); return; }
	if(THIS_TABLE->table_full) { printf("[ERR] Unable to add entry to table [%s] in database [%s], the table has no free entries.\n"
					    , THIS_TABLE->tbl_name, dbs[db_index]->db_name); return; }
        char* struct_mem = THIS_TABLE->table_ptr;
	struct_mem += (THIS_TABLE->next_free_row * ( (THIS_TABLE->columns)  * ROW_ENTRY_SIZ ));

	/* Now struct_mem is at the memory location of the desired row in the table. */
	for(size_t i = 0; i < THIS_TABLE->columns; ++i){
                memcpy((void*)struct_mem, (void*)contents_address, ROW_ENTRY_SIZ);
                struct_mem += ROW_ENTRY_SIZ;
                contents_address += ROW_ENTRY_SIZ;
        }
	++THIS_TABLE->active_entries;

        printf(
                "[OK] Edited entry [%lu] in table [%s] in database [%s]\n",
                THIS_TABLE->next_free_row,
                THIS_TABLE->tbl_name,
                dbs[db_index]->db_name
              );


	++THIS_TABLE->next_free_row;
	size_t free_row_i = THIS_TABLE->next_free_row;
	for(char* next_row_addr = struct_mem ; ; next_row_addr += THIS_TABLE->columns * ROW_ENTRY_SIZ){
		if( 
		    (*next_row_addr) 
		    && 
		    ( free_row_i < INITIAL_ROWS) 
		  ) 
		{ ++free_row_i; continue; }
		else { break; }	
	}
	/* Two possible outcomes from the for loop: Either all entries were full so free_row_i = INITIAL_ROWS */
	/* 				            Or it found an entry that begins with a 0 byte, so free_row_i < INITIAL_ROWS */

	if( free_row_i < INITIAL_ROWS ){ THIS_TABLE->next_free_row = free_row_i; }
	else { THIS_TABLE->table_full = 1; }
}

static void Print_Table(size_t db_index, size_t tbl_index){
	print_red();
	printf("\n\n******************** PRINTING TABLE [%s] in database [%s] *********************\n\n",
		THIS_TABLE->tbl_name,
		dbs[db_index]->db_name	
			);
	print_reset();
	char* struct_mem = THIS_TABLE->table_ptr;
	//Memory size of each row including delimiters " | ", newline and terminating null byte.
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
	printf("\n\n********************** END OF TABLE *********************************\n\n");
	print_reset();
}	

void Delete_Row(size_t db_index, size_t tbl_index, size_t row_index){
	memset(
		THIS_TABLE->table_ptr + (row_index * THIS_TABLE->columns * ROW_ENTRY_SIZ)
		, 0x0
		, THIS_TABLE->columns * ROW_ENTRY_SIZ
	      );
	if(THIS_TABLE->table_full) {
	       	THIS_TABLE->next_free_row = row_index;  
		THIS_TABLE->table_full = 0;
	}
	else if(row_index < THIS_TABLE->next_free_row){ THIS_TABLE->next_free_row = row_index; } 
	printf("[OK] Erased row [%lu] of table [%s] in database [%s]\n", row_index, THIS_TABLE->tbl_name, dbs[db_index]->db_name);
}

void Delete_Table(size_t db_index, size_t tbl_index){
	for(size_t i = 0; i < INITIAL_ROWS; ++i){ Delete_Row(db_index, tbl_index, i); }	
	free(THIS_TABLE->table_ptr);
	free(THIS_TABLE);
}

void Delete_Database(size_t db_index){
	for(size_t i = 0; i < 64; ++i){ Delete_Table(db_index, i); }
	free(dbs[db_index]);
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
				{ print_red(); printf("[ERR] Could not write table name [%s] in database [%s] to savefile.\n", 
						dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); print_reset(); }
			else{ printf("[OK] Wrote to savefile %lu bytes of table name [%s] in database [%s].\n",
				       	bytes_written, dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); }

                        if(!(bytes_written = fwrite(&dbs[a]->tables[b]->next_free_row, 1, 8, savefile)))
                                { print_red(); printf("[ERR] Could not write next free row of table [%s] in database [%s] to savefile.\n",
                                                dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Wrote to savefile next free row of table [%s] in database [%s].\n",
                                        	dbs[a]->tables[b]->tbl_name, dbs[a]->db_name); }

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
	size_t next_free_row = 0;
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
                                { print_red(); printf("[ERR] Could not read table name of table [%s] in database [%s] from savefile.\n", buffer, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Read table name [%s] in database [%s] from savefile.\n", buffer, dbs[a]->db_name); }

                        if(!(bytes_read = fread(&next_free_row, 1, 8, savefile)))
                                { print_red(); printf("[ERR] Could not read next free row index of table [%s] in database [%s] from savefile.\n", buffer, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Read next free row index of table [%s] in database [%s] from savefile.\n", buffer, dbs[a]->db_name); }

                        if(!(bytes_read = fread(&table_full, 1, 1, savefile)))
                                { print_red(); printf("[ERR] Could not read the table_full flag of table [%s] in database [%s] from savefile.\n", buffer, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Read table_full flag of table [%s] in database [%s] from savefile.\n", buffer, dbs[a]->db_name); }


                        if(!(bytes_read = fread(buffer + 64, 1, 8, savefile)))
                                { print_red(); printf("[ERR] Could not read # of entries in table [%s] in database [%s] from savefile.\n", buffer, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Read from savefile # of entries in table [%s] in database [%s]: %lu entries.\n", buffer, dbs[a]->db_name, (size_t)(*(buffer + 64))); }

                        if(!(bytes_read = fread(buffer + 72, 1, 8, savefile)))
                                { print_red(); printf("[ERR] Could not read # of columns in table [%s] in database [%s] from savefile.\n", buffer, dbs[a]->db_name); print_reset(); }
                        else{ printf("[OK] Read from savefile # of columns in table [%s] in database [%s]: %lu columns.\n", buffer, dbs[a]->db_name, (size_t)(*(buffer + 72))); }
			
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
int main(){
	memset(dbs, 0x0, 64*sizeof(struct database*));
	
	Load_System();
	Print_Table(0, 0);
	Print_Table(0, 1);
	Save_System();
	Load_System();
	Print_Table(0, 0);
	Print_Table(0, 1);


/* 
	Create_Database("Veterinarian");
	Create_Table(0, 0, 4, "Pets", 0);

	char col_buffer[10 * ROW_ENTRY_SIZ];
	size_t entry_siz_aux = 0;

        //  Usually this row string will come from a system source such
        //  as a web server on the databases' socket buffer (w/o entry #).
        //  Its bytes will be printable and a Line Feed will divide each entry.
        //  A null byte will be guaranteed to terminate the string.
	//  Each thread will have exactly one input string row buffer.
        //  For this example we just initialize a bunch of strings to test the macro.
	char *row0 = malloc(128), *row1 = malloc(128), *row2 = malloc(128), *row3 = malloc(128);
	strcpy(row0, "ID Name Owner Illness\0");
	strcpy(row1, "1 Sarah Natalia Tubercolosis\0");
	strcpy(row2, "2 Xerox Taratatulia Lung_Disease\0");
	strcpy(row3, "3 Masha Stefanov Code_Problems");
		
        CONSTRUCT_ROW_BUFFER(col_buffer, row0, 4, entry_siz_aux);     
	Add_Row(0, 0, col_buffer);
	
	CONSTRUCT_ROW_BUFFER(col_buffer, row1, 4, entry_siz_aux);
	Add_Row(0, 0, col_buffer);

	CONSTRUCT_ROW_BUFFER(col_buffer, row2, 4, entry_siz_aux);
	Add_Row(0, 0, col_buffer);

	CONSTRUCT_ROW_BUFFER(col_buffer, row3, 4, entry_siz_aux);
	Add_Row(0, 0, col_buffer);

	Print_Table(0, 0);

	Create_Table(0, 0, 4, "Doctors", 0);

	Save_System();

*/
	return 0;

}






























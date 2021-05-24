#include "cApp.h"
#include "DBMS Functions + Classes.h"

extern cMain** ptr_obj;
std::string SQL_Response = "";

//***************************************************************************************************
//********************************** MAIN DBMS CLASSES FOLLOW ***************************************
//***************************************************************************************************
#include <string>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>

using namespace std;

static size_t max_len = 16;


	Table::Table(string& s, vector<string>& v) : table_name(s) {
		v.insert(v.begin(), 1, "ID");
		T.push_back(v);
	}

	//Insert an entry to the table with provided values [INSERT INTO name VALUES vals]
	void Table::Insert(vector<string>& v) {
		for (size_t i = 0; i < v.size(); ++i) {
			if (v[i].size() > max_len) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: An element you tried entering is too long. Max length of an entry: 16");

				return;
			}
		}
		if (v.size() == T[0].size() - 1) {
			v.insert(v.begin(), 1, to_string(primary_key));
			++primary_key;
			++number_of_entries;
			T.push_back(v);
		}
		else if (v.size() < T[0].size() - 1) {
			(*ptr_obj)->m_err_output->AppendString("ERROR: Too few arguments supplied (" + _(to_string(v.size())) + _("). Please supply ")
				+ _(to_string((T[0].size() - 1))) + _(" arguments in order to add a row to this table."));
		}
		else if (v.size() > T[0].size() - 1) {
			(*ptr_obj)->m_err_output->AppendString("ERROR: Too many arguments supplied (" + _(to_string(v.size())) + _("). Please supply ")
				+ _(to_string((T[0].size() - 1))) + _(" arguments in order to add a row to this table."));
		}
	}

	//On delete, number_of_entries decrements, but primary_key does not.
	void Table::Delete(unsigned& key) {
		bool found = false;
		string key_str = to_string(key);
		unsigned position_index = 0;
		for (unsigned i = 1; i < T.size(); ++i) {
			if (T[i][0] == key_str) {
				found = true;
				position_index = i;
				break;
			}
		}
		if (found) {
			(*ptr_obj)->m_err_output->AppendString("SUCCESS: Erasing the row at index: " +
				_(to_string(position_index)) + _(" (primary key: ") + _(to_string(key)) + _(")"));
			T.erase(T.begin() + position_index);
			--number_of_entries;
		}
		else {
			(*ptr_obj)->m_err_output->AppendString("ERROR: An entry inside the table with the provided primary key could not be located.");
			return;
		}
	}

	void Table::Alter(size_t& row_index, size_t& column_index, string& s) {
		if (T.size() > row_index && T[row_index].size() > column_index && s.size() <= 16) {
			(*ptr_obj)->m_err_output->AppendString("SUCCESS: Setting T[" + _(to_string(row_index))
				+ _("][") + _(to_string(column_index)) + _("] to ") + _(s));
			T[row_index][column_index] = s;
		}
		else if (s.size() >= max_len) {
			(*ptr_obj)->m_err_output->AppendString("ERROR: The element you tried entering is too long. Max length of an entry: 16");
			return;
		}
		else if (T.size() <= row_index || T[row_index].size() <= column_index) {
			(*ptr_obj)->m_err_output->AppendString("ERROR: The position at which you attempted to modify an element could not be located in this table.");
			return;
		}
	}

	//Print elements of a table [SELECT-[* || string_list]-FROM-table_name-WHERE-[condition(s)]].
	void Table::Print(vector<unsigned>& VC, vector<unsigned>& VR) {
		size_t spaces = 0, half_spaces = 0;
		string row_str = "";
		bool odd = false, found = true;
		for (unsigned i = 0; i < VR.size(); ++i) {
			for (unsigned j = 0; j < VC.size(); ++j) {
				if (T.size() > VR[i] && T[VR[i]].size() > VC[j]) {
					spaces = 16 - T[VR[i]][VC[j]].size();
					if ((spaces % 2) == 1) { odd = true; --spaces; }
					half_spaces = spaces / 2;
					while (half_spaces > 0) {
						row_str += " ";
						--half_spaces;
					}
					row_str += T[VR[i]][VC[j]];
					half_spaces = spaces / 2;
					while (half_spaces > 0) {
						row_str += " ";
						--half_spaces;
					}
					if (odd) { row_str += " "; }
					row_str += "|";
					odd = false;
				}
				else { (*ptr_obj)->m_err_output->AppendString("ERROR: Element to print not located."); found = false; }
			}
			(*ptr_obj)->m_main_output->AppendString(row_str);
			row_str = "";
		}
		if (found) { (*ptr_obj)->m_main_output->AppendString(""); }
	}

	//Show the column names of the table (the core row)
	void Table::ShowAttributes(void) {
		string row_str = "";
		row_str += ("Attributes of table '" + table_name + "': ");
		for (size_t i = 0; i < T[0].size() - 1; ++i) {
			row_str += (T[0][i] + ", ");
		}
		row_str += T[0][T[0].size() - 1];
		(*ptr_obj)->m_main_output->AppendString(row_str);
	}




	int Database::NumberOfTables(void) const {
		return (int)tables.size();
	}

	Database::Database(string& s, vector<Table>& v) : db_name(s), tables(v) {};
	Database::Database(string& s) : db_name(s) {
		tables = {};
	};

	void Database::AddTable(Table& T) { tables.push_back(T); }

	void Database::RemoveTable(string& s) {
		bool found = false;
		unsigned position = 0;
		for (unsigned i = 0; i < tables.size(); ++i) {
			if ((tables[i]).table_name == s) {
				found = true;
				position = i;
				break;
			}
		}
		if (found) {
			(*ptr_obj)->m_err_output->AppendString("SUCCESS: Removing table '" + _(tables[position].table_name) + _("'"));
			tables.erase(tables.begin() + position);
		}
		else {
			(*ptr_obj)->m_err_output->AppendString("ERROR: A table with the supplied name could not be located.");
		}
	}

//***************************************************************************************************
//********************************* MAIN DBMS FUNCTIONS FOLLOW **************************************
//***************************************************************************************************

using namespace std;

vector<Database> databases = {};

vector<string> SeparateAllWords(string s) {
	vector<string> output_vec;
	string str = "";
	for (size_t i = 0; i < s.size(); ++i) {
		if (i == s.size() - 1) { str.push_back(s[i]), output_vec.push_back(str); break; }
		if (s[i] != '-') {
			str.push_back(s[i]);
		}
		else {
			output_vec.push_back(str);
			str = "";
		}
	}
	return output_vec;
}

bool ContainsOnlyDigits(string s) {
	for (size_t i = 0; i < s.size(); ++i) {
		if(!(s[i] > 47 && s[i] < 58)){return false;}
	}
	return true;
}

bool CheckCondition(string operand1, string Operator, string operand2) {
	bool holds = true, numeric = false;
	if (ContainsOnlyDigits(operand1) && ContainsOnlyDigits(operand2)) {
		numeric = true;
	}
	if (numeric) {
		double operand_a = stod(operand1), operand_b = stod(operand2);
		if (Operator == "<") {
			if (operand_a >= operand_b) {
				holds = false;
			}
		}
		else if (Operator == "=") {
			if (operand_a != operand_b) {
				holds = false;
			}
		}
		else if (Operator == ">") {
			if (operand_a <= operand_b) {
				holds = false;
			}
		}
	}
	else {
		if (operand1 != operand2) {
			holds = false;
		}
	}
	return holds;
}

vector<string> SeparateCondition(string s) {
	vector<string> output_vec = {};
	string part = "";
	size_t index = 0;
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] != '=' && s[i] != '>' && s[i] != '<') {
			part.push_back(s[i]);
		}
		else {
			output_vec.push_back(part);
			part = "";
			if (s[i] == '=') {
				part = "=";
			}
			else if (s[i] == '>') {
				part = ">";
			}
			else if (s[i] == '<') {
				part = "<";
			}
			output_vec.push_back(part);
			part = "";
			for (size_t j = i + 1; j < s.size(); ++j) {
				part.push_back(s[j]);
			}
			output_vec.push_back(part);
			break;
		}
	}
	return output_vec;
}

bool SQL_Command_Interpreter(string& command) {
	vector<string> cmd = SeparateAllWords(command);
	size_t db_pos = 0, tbl_pos = 0, row_pos = 0, col_pos = 0;
	bool found = false;
	if (cmd[0] == "SHOW_ALL") {
		(*ptr_obj)->m_main_output->AppendString("Outputting database names...");
		for (size_t i = 0; i < databases.size(); ++i) {
			(*ptr_obj)->m_main_output->AppendString(databases[i].db_name);
		}
		return true;
	}
	if (cmd[0] == "CLEAR_ERR") {
		(*ptr_obj)->m_err_output->Clear();
		return true;
	}
	if (cmd[0] == "CLEAR_MAIN") {
		(*ptr_obj)->m_main_output->Clear();
		return true;
	}
	if ((!(cmd[0] == "CREATE"))) {
		for (size_t i = 0; i < databases.size(); ++i) {
			if (databases[i].db_name == cmd[0]) {
				found = true;
				db_pos = i;
				break;
			}
		}
		if (!found) {
			(*ptr_obj)->m_err_output->AppendString("ERROR: A database with the supplied name does not exist.");
			return false;
		}
	}
	//Code for creating a database, BUT NOT for creating a table, goes here.
	else {
		if(cmd[1] != "DB"){(*ptr_obj)->m_err_output->AppendString("ERROR: Invalid command to create a database. Syntax: CREATE-DB-dbname-num_tables-tbl1_name-num_cols-col1-col2-tbl2_name-num_cols-..."); return false;}
		found = false;
		if (cmd.size() < 4) {
			(*ptr_obj)->m_err_output->AppendString("ERROR: Command to create a database is too short. Minimum size: 4");
			return false;
		}
		string db_name = cmd[2], tbl_name = "";
		vector<Table> vec_tbls = {};
		vector<string> columns = {};
		if (stoi(cmd[3]) > 0) {
			int tbls = stoi(cmd[3]), pos_counter = 4, cols = 0;
			if (cmd.size() < ((3 * tbls) + 4)) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: Command to create the database is too short. Minimum required size: "
					+ _(to_string(((3 * tbls) + 4))));
				return false;
			}
			for (int i = 0; i < tbls; ++i) {
				tbl_name = cmd[pos_counter];
				++pos_counter;
				cols = stoi(cmd[pos_counter]);
				for (int j = 0; j < cols; ++j) {
					++pos_counter;
					columns.push_back(cmd[pos_counter]);
				}
				Table tbl_from_command(tbl_name, columns);
				vec_tbls.push_back(tbl_from_command);
				columns = {};
				++pos_counter;
			}
			Database db(db_name, vec_tbls);
			databases.push_back(db);
			(*ptr_obj)->m_err_output->AppendString("SUCCESS: Added the database to the system!");
			return true;
		}
		else {
			Database db(db_name);
			databases.push_back(db);
			(*ptr_obj)->m_err_output->AppendString("SUCCESS: Added the database to the system!");
			return true;
		}
	}
	//Add a table to an existing database.
	if (cmd.size() > 1) {


		if (cmd[1] == "ADD") {
			if (cmd.size() < 5) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: Command to add a table to a database is too short.Minimum size : 5");
				return false;
			}
			size_t pos_counter = 2;
			string tbl_name = cmd[pos_counter];
			++pos_counter;
			int cols = stoi(cmd[pos_counter]);
			if (cmd.size() != (cols + 4)) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: Command to add the table to the database is of invalid length. Required length: "
					+ _(to_string(cols + 4)));
				return false;
			}
			++pos_counter;
			vector<string> col_names = {};
			for (size_t i = 0; i < cols; ++i) {
				col_names.push_back(cmd[pos_counter]);
				++pos_counter;
			}
			Table tbl(tbl_name, col_names);
			databases[db_pos].AddTable(tbl);
			(*ptr_obj)->m_err_output->AppendString("SUCCESS: Added the table " + _(tbl.table_name) + _(" to database ") + _(databases[db_pos].db_name));
			return true;
		}
		found = false;
		if (cmd[1] == "SHOW_ALL") {
			if (databases[db_pos].tables.size() == 0) {
				(*ptr_obj)->m_main_output->AppendString("The database is empty.");
				return true;
			}
			else {
				(*ptr_obj)->m_main_output->AppendString("Outputting tables names...");
				for (size_t i = 0; i < databases[db_pos].tables.size(); ++i) {
					(*ptr_obj)->m_main_output->AppendString(databases[db_pos].tables[i].table_name);
				}
				return true;
			}
		}
		if (cmd.size() > 2 && cmd[2] == "SHOW_ALL") {
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables.size(); ++i) {
				if (cmd[1] == databases[db_pos].tables[i].table_name) {
					(*ptr_obj)->m_main_output->AppendString("Outputting table column names...");
					for (size_t j = 0; j < databases[db_pos].tables[i].T[0].size(); ++j) {
						(*ptr_obj)->m_main_output->AppendString(databases[db_pos].tables[i].T[0][j]);
					}
					return true;
				}
			}
			(*ptr_obj)->m_err_output->AppendString("ERROR: A table could not be located within this database.");
			return false;


		}
		//DB_NAME-DELETE-TABLE-TBL_NAME
		if (cmd.size() > 2 && cmd[1] == "DELETE" && cmd[2] == "TABLE") {
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables.size(); ++i) {
				if (cmd[3] == databases[db_pos].tables[i].table_name) {
					found = true;
					databases[db_pos].tables.erase(databases[db_pos].tables.begin() + i);
					(*ptr_obj)->m_err_output->AppendString("SUCCESS: Deleted the table from the database.");
					return true;
				}
			}
			if (!found) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: A table could not be located within this database.");
				return false;
			}
		}
		//DELETE-DB_NAME
		if (cmd[1] == "DELETE" && cmd.size() < 3) {
			databases.erase(databases.begin() + db_pos);
			(*ptr_obj)->m_err_output->AppendString("SUCCESS: Deleted the database from the systen.");
			return true;
		}
		found = false;
		//Insert an entry row into a table of an existing database.
		if (cmd[1] == "INSERT") {
			if (cmd.size() < 6) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: Command to insert an entry row into a table is too short. Minimum size: 6");
				return false;
			}
			size_t pos_counter = 3;
			string tbl_name = cmd[pos_counter];
			//Locate the table within the database.
			bool found = false;
			for (size_t i = 0; i < databases[db_pos].tables.size(); ++i) {
				if (databases[db_pos].tables[i].table_name == tbl_name) {
					found = true;
					tbl_pos = i;
					break;
				}
			}
			if (!found) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: A table with the supplied name does not exist within this database.");
				return false;
			}

			pos_counter += 2;
			size_t req_size = databases[db_pos].tables[tbl_pos].T[0].size() - 1;
			if ((cmd.size() - 5) != req_size) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: Command to insert an entry row into a table is of invalid size. Required size: "
					+ _(to_string((databases[db_pos].tables[tbl_pos].T[0].size() - 1) + 5)));
				return false;
			}
			vector<string> input = {};
			for (int i = 0; i < req_size; ++i) {
				input.push_back(cmd[pos_counter]);
				++pos_counter;
			}
			(*ptr_obj)->m_err_output->AppendString("SUCCESS: Added an entry to table " + _(databases[db_pos].tables[tbl_pos].table_name) + _(" in database ") + _(databases[db_pos].db_name));
			databases[db_pos].tables[tbl_pos].Insert(input);
			return true;
		}
		//Second one is intended for systems using the database.
		if (cmd[1] == "DELETE_ENTRY" || cmd[1] == "CHECK") {
			int checking = 0;
			if(cmd[1] == "CHECK"){checking = 1;}
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables.size(); ++i) {
				if (databases[db_pos].tables[i].table_name == cmd[2]) {
					found = true;
					tbl_pos = i;
					break;
				}
			}
			if (!found) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: A table with the supplied name does not exist within this database.");
				if(checking){SQL_Response = "no";}
				return false;
			}
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables[tbl_pos].T[0].size(); ++i) {
				if (databases[db_pos].tables[tbl_pos].T[0][i] == cmd[3]) {
					found = true;
					col_pos = i;
					break;
				}
			}
			if (!found) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: A column with the name " + _(cmd[4]) + _(" does not exist within this table."));
				if(checking){SQL_Response = "no";}
				return false;
			}
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables[tbl_pos].T.size(); ++i) {
				if (databases[db_pos].tables[tbl_pos].T[i][col_pos] == cmd[4]) {
					found = true;
					if(cmd[1] == "DELETE_ENTRY"){
						databases[db_pos].tables[tbl_pos].T.erase(databases[db_pos].tables[tbl_pos].T.begin() + i);
						(*ptr_obj)->m_err_output->AppendString("SUCCESS: Deleted an entry from the table in this database.");
					}
					else if(checking){
						SQL_Response = "yes";
						for(size_t j = 5; j < cmd.size(); ++j){
							for(size_t k = 0; k < databases[db_pos].tables[tbl_pos].T[0].size(); ++k){
								if(databases[db_pos].tables[tbl_pos].T[0][k] == cmd[j]){
									SQL_Response += ("-" + databases[db_pos].tables[tbl_pos].T[i][k]);
								}
							}
						}
					}
				}
			}
			if (!found) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: An entry with the specified value for this particular attribute does not exist within this table.");
				if(checking){SQL_Response = "no";}
				return false;
			} else{return true;}
		}
		if (cmd[1] == "ALTER") {
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables.size(); ++i) {
				if (databases[db_pos].tables[i].table_name == cmd[2]) {
					found = true;
					tbl_pos = i;
					break;
				}
			}
			if (!found) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: A table with the supplied name does not exist within this database.");
				return false;
			}
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables[tbl_pos].T[0].size(); ++i) {
				if (databases[db_pos].tables[tbl_pos].T[0][i] == cmd[4]) {
					found = true;
					col_pos = i;
					break;
				}
			}
			if (!found) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: A column with the name " + _(cmd[4]) + _(" does not exist within this table."));
				return false;
			}
			vector<size_t> row_positions = {};
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables[tbl_pos].T.size(); ++i) {
				if (databases[db_pos].tables[tbl_pos].T[i][col_pos] == cmd[5]) {
					found = true;
					row_positions.push_back(i);
				}
			}
			if (!found) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: An entry with the specified value for this particular attribute does not exist within this table.");
				return false;
			}
			found = false;

			for (size_t i = 0; i < databases[db_pos].tables[tbl_pos].T[0].size(); ++i) {
				if (databases[db_pos].tables[tbl_pos].T[0][i] == cmd[6]) {
					found = true;
					col_pos = i;
					break;
				}
			}
			if (!found) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: A column with the name " + _(cmd[6]) + _(" does not exist within this table."));
				return false;
			}
			if (cmd[7].size() <= 16) {
				for (size_t i = 0; i < row_positions.size(); ++i) {
					databases[db_pos].tables[tbl_pos].Alter(row_positions[i], col_pos, cmd[7]);
					(*ptr_obj)->m_err_output->AppendString("SUCCESS: Alteration of table " + _(cmd[2])
						+ _(" within database ") + _(cmd[0]) + _(" has been successful."));
				}
				return true;
			}
			else {
				(*ptr_obj)->m_err_output->AppendString("ERROR: The data item you tried to alter one of the tables' entries' attribute with is too long. Max length: 16");
				return false;
			}
		}
		if (cmd[1] == "SELECT") {
			vector<vector<string>> conds = {};
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables.size(); ++i) {
				if (databases[db_pos].tables[i].table_name == cmd[3]) {
					found = true;
					tbl_pos = i;
					break;
				}
			}
			if (!found) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: A table with the supplied name does not exist within this database.");
				return false;
			}
			if (databases[db_pos].tables[tbl_pos].T.size() == 1) {
				(*ptr_obj)->m_err_output->AppendString("ERROR: The table you are trying to select items from is empty.");
				return false;
			}
			found = false;
			vector<unsigned int> vecPOScol = {}, vecPOSrow = {};
			size_t pos = 4;
			if (cmd[4] == "*") {
				for (size_t i = 0; i < databases[db_pos].tables[tbl_pos].T[0].size(); ++i) {
					vecPOScol.push_back(i);
				}
				++pos;
			}
			else {
				for (size_t i = 4; cmd[i] != "WHERE"; ++i) {
					for (size_t j = 0; j < databases[db_pos].tables[tbl_pos].T[0].size(); ++j) {
						if (databases[db_pos].tables[tbl_pos].T[0][j] == cmd[i]) {
							vecPOScol.push_back(j);
							break;
						}
						if (j == databases[db_pos].tables[tbl_pos].T[0].size() - 1) {
							(*ptr_obj)->m_err_output->AppendString("ERROR: A column with the name "
								+ _(cmd[i]) + _(" does not exist within this table."));
							break;
						}
					}
					++pos;
					if ((i + 1) >= cmd.size()) { break; }
				}
			}
			++pos;
			vector<size_t> cond_row_pos = {};
			vector<string> dummy_vec = {};
			for (size_t i = pos; i < cmd.size(); ++i) {
				conds.push_back(SeparateCondition(cmd[i]));
			}
			vector<size_t> invalid_conds = {};
			bool invalid = false;
			for (size_t i = 0; i < conds.size(); ++i) {
				invalid = false;
				for (size_t j = 0; j < databases[db_pos].tables[tbl_pos].T[0].size(); ++j) {
					if (databases[db_pos].tables[tbl_pos].T[0][j] == conds[i][0]) {
						pos = j;
						cond_row_pos.push_back(j);
						break;
					}
					if (j == databases[db_pos].tables[tbl_pos].T[0].size() - 1) {
						(*ptr_obj)->m_err_output->AppendString("ERROR: A column with the name " + _(conds[i][0]) + _(" does not exist within this table."));
						invalid_conds.push_back(i); 
						invalid = true; 
						break;
					}
				}
				if ((conds[i][1] == ">" || conds[i][1] == "<") && (!invalid)) {
					if (!(ContainsOnlyDigits(databases[db_pos].tables[tbl_pos].T[1][pos]))) {
						(*ptr_obj)->m_err_output->AppendString("ERROR: Cannot use comparison operators on non-numeric items. Invalid condition: "
							+ _(conds[i][0]) + _(" ") + _(conds[i][1]) + _(" ") + _(conds[i][2]));
						invalid_conds.push_back(i);
					}
				}
			}
			bool fill = true, valid = false, invalid_conds_exist = false, check1 = false;
			for (size_t i = 1; i < databases[db_pos].tables[tbl_pos].T.size(); ++i) {
				valid = false;
				invalid_conds_exist = false;
				for (size_t j = 0; j < conds.size(); ++j) {
					if (invalid_conds.size() > 0) {
						invalid_conds_exist = true;
						if ((find(invalid_conds.begin(), invalid_conds.end(), j) == invalid_conds.end()))
						{
							valid = true;
						}
					}
					if (invalid_conds_exist == false || ((invalid_conds_exist == true) && (valid == true))) {
						check1 = CheckCondition(databases[db_pos].tables[tbl_pos].T[i][cond_row_pos[j]], conds[j][1], conds[j][2]);
						if (check1 == false) {
							fill = false;
						}
					}
				}
				if (fill == true) {
					vecPOSrow.push_back(i);
				}
				else {
					fill = true;
				}
			}
			databases[db_pos].tables[tbl_pos].Print(vecPOScol, vecPOSrow);
			vecPOSrow = {};
			vecPOScol = {};
			invalid_conds = {};
			cond_row_pos = {};
			conds = {};
			dummy_vec = {};
			return true;
		}
	}
	return true;
}

void SaveSystem(bool out_msg) {
	if ((out_msg) && (ptr_obj) && (*ptr_obj)) {(*ptr_obj)->m_err_output->AppendString("System saving...");}
	ofstream myfile;
	myfile.open("SavedDatabases.txt");
	for (size_t i = 0; i < databases.size(); ++i) {
		myfile << "*\n";
		myfile << databases[i].db_name << "\n";
		for (size_t j = 0; j < databases[i].tables.size(); ++j) {
			myfile << "#\n";
			myfile << databases[i].tables[j].table_name << "\n";
			for (size_t k = 0; k < databases[i].tables[j].T.size(); ++k) {
				for (size_t z = 0; z < databases[i].tables[j].T[k].size(); ++z) {
					if (!(z == databases[i].tables[j].T[k].size() - 1)) {
						myfile << databases[i].tables[j].T[k][z] << "-";
					}
					else {
						myfile << databases[i].tables[j].T[k][z];
					}
				}
				myfile << "\n";
			}
		}
		myfile << "TBL_END\n";
	}
	myfile << "SYS_END\n";
	myfile.close();
	myfile.open("CommandHistory.txt", fstream::app);
	time_t now = time(0);
	string dt = ctime(&now);
	myfile << "ON " << dt << "\n";
	if ((ptr_obj) && (*ptr_obj)) {
		wxArrayString hist = (*ptr_obj)->m_history_txt->GetStrings();
		for (size_t i = 0; i < hist.size(); ++i) {
			myfile << hist[i] << "\n";
		}
	}
	myfile << "\n\n";
	myfile.close();
	if ((out_msg) && (ptr_obj) && (*ptr_obj)) {(*ptr_obj)->m_err_output->AppendString("System saved.");}
	return;
}

void LoadSystem(bool out_msg) {
	string line, db_name = "", tbl_name = "";
	int ID_state = 0;
	bool db_nameb = false, tbl_nameb = false, found_db = false;
	vector<vector<string>> rows = {};
	vector<string> row_entries = {};
	vector<Table> tbls = {};
	if ((out_msg) && (ptr_obj) && (*ptr_obj)) {(*ptr_obj)->m_err_output->AppendString("System loading...");}
	ifstream myfile1("SavedDatabases.txt");
	if (myfile1.is_open())
	{
		while (getline(myfile1, line))
		{
			if((found_db) && (line != "*") && (line != "SYS_END")){continue;}
			else if (found_db && line == "*") {
				found_db = false;
				continue;
			}
			else if (found_db && line == "SYS_END") {
				if ((out_msg) && (ptr_obj) && (*ptr_obj)) {(*ptr_obj)->m_err_output->AppendString("System loaded successfully.");}
				break;
			}
			if (db_nameb) {
				db_name = line;
				for (size_t i = 0; i < databases.size(); ++i) {
					if (db_name == databases[i].db_name) {
						found_db = true;
						break;
					}
				}
				if (found_db) {continue;}
				db_nameb = false;
			}
			else if (tbl_nameb) {
				tbl_nameb = false;
				tbl_name = line;
			}
			else if (line == "*") {
				db_nameb = true;
			}
			else if (line == "#") {
				if (!rows.size() == 0) {
					Table tbl(tbl_name, rows[0]);
					tbl.T[0].erase(tbl.T[0].begin());
					for (size_t i = 1; i < rows.size(); ++i) {
						tbl.T.push_back(rows[i]);
					}
					tbl.number_of_entries = tbl.T.size();
					if (tbl.T.size() == 1) {
						tbl.primary_key = 0;
					}
					else {
						tbl.primary_key = stoull(tbl.T[tbl.T.size() - 1][0]) + 1;
					}
					tbls.push_back(tbl);
					rows = {};
				}
				tbl_nameb = true;
			}
			else if (line == "TBL_END") {
				Table tbl(tbl_name, rows[0]);
				tbl.T[0].erase(tbl.T[0].begin());
				for (size_t i = 1; i < rows.size(); ++i) {
					tbl.T.push_back(rows[i]);
				}
				tbl.number_of_entries = tbl.T.size();
				if (tbl.T.size() == 1) {
					tbl.primary_key = 0;
				}
				else {
					tbl.primary_key = stoull(tbl.T[tbl.T.size() - 1][0]) + 1;
				}
				tbls.push_back(tbl);
				rows = {};
				Database in_db(db_name, tbls);
				databases.push_back(in_db);
				tbls = {};
			}
			//The end of reading.
			else if (line == "SYS_END") {
				if ((out_msg) && (ptr_obj) && (*ptr_obj)) {(*ptr_obj)->m_err_output->AppendString("System loaded successfully.");}
				break;
			}
			else {
				rows.push_back(SeparateAllWords(line));
			}
		}
		myfile1.close();
	}
}


void UpdateDatabase(string& filename) {
	LoadSystem(false);

	char buf[128];
	int fd, cl, rc;
	size_t sql_response_siz;
	char* socket_path = "\0hidden";
	struct sockaddr_un addr;
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	*addr.sun_path = '\0';
	strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
	bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	listen(fd, 5);
	std::string req = "";
	while(1){
		memset(buf, 0x0, sizeof(buf));
		req = "";
		SQL_Response = "";
		cl = accept(fd, NULL, NULL);
		if((rc = read(cl, buf, sizeof(buf))) > 0){
			printf("The web server sent me this: %s\n", buf);
			for(size_t i = 0; buf[i] != '\0'; ++i){req.push_back(buf[i]);} 
			SQL_Command_Interpreter(req);
			memset(buf, 0x0, 128);
			sql_response_siz = SQL_Response.length();
			for(size_t i = 0; i < sql_response_siz; ++i){buf[i] = SQL_Response[i];}
		}
		printf("About to send the following sql response to the web server: %s\n", buf);
		write(cl, buf, strlen(buf));
		if(rc == -1){ perror("read error\n"); exit(-1);}
		else if(rc == 0){printf("EOF\n");close(cl);}
	}
	return;
}

//*********************************************************************************************************************
//************************************************* END OF MAIN DBMS FUNCTIONS ****************************************
//*********************************************************************************************************************

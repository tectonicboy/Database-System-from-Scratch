#include <string>
#include <cmath>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <fstream>

#include "Database.cpp"

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
	//cout << "Separated the command into a vector of strings of size: " << output_vec.size() << "\n";
	return output_vec;
}

bool ContainsOnlyDigits(string s) {
	bool b = true;
	for (size_t i = 0; i < s.size(); ++i) {
		if ((s[i] != '0') && (s[i] != '1') && (s[i] != '2') && (s[i] != '3') && (s[i] != '4') && (s[i] != '5')
			&& (s[i] != '6') && (s[i] != '7') && (s[i] != '8') && (s[i] != '9'))
		{
			b = false;
		}
	}
	return b;
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
	//cout << "Entered the SeparateCondition function with string: " << s << "\n";
	vector<string> output_vec = {};
	string part = "";
	size_t index = 0;
	for (size_t i = 0; i < s.size(); ++i) {
		//cout << "Entered for loop. i = " << i << ". s[i] = " << s[i] << "\n";
		if (s[i] != '=' && s[i] != '>' && s[i] != '<') {
			part.push_back(s[i]);
		}
		else {
			//cout << "Encountered " << s[i] << ". Pushing " << part << "\n";
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
			//cout << "Encountered " << s[i] << ". Pushing " << part << "\n";
			output_vec.push_back(part);
			part = "";
			for (size_t j = i + 1; j < s.size(); ++j) {
				part.push_back(s[j]);
			}
			//cout << "Pushing " << part << "\n";
			output_vec.push_back(part);
			break;
		}
	}

	//cout << "Separated the condition into: [0] = " << output_vec[0] << ", [1] = " << output_vec[1] << ", [2] = " << output_vec[2] << "\n";
	return output_vec;
}


void SQL_Command_Interpreter(string& command) {
	vector<string> cmd = SeparateAllWords(command);
	size_t db_pos = 0, tbl_pos = 0, row_pos = 0, col_pos = 0;
	bool found = false;
	if (cmd[0] == "ls") {
		cout << "Listing valid commands... {} indicates a non-mandatory part of a command.\n\n";
		cout << "SELECT Statement:\n";
		cout << "DB_name-SELECT-FROM-tbl_name-[list_of_-seprated_column_names OR *]{-WHERE-[list_of_-seprated_conditions]}\n";
		cout << "Examples:\n";
		cout << "Doctors_Database-SELECT-FROM-doctors-name-company-WHERE-ID>0\n";
		cout << "Doctors_Database-SELECT-FROM-security_staff-*-WHERE-ID>0-name=Stacy\n";
		cout << "Doctors_Database-SELECT-FROM-security_staff-*\n\n\n";

		cout << "ALTER Statement:\n";
		cout << "DB_name-ALTER-tbl_name-WHERE-column_name1-existing_value-column_name2-new_value\n";
		cout << "Example:\n";
		cout << "Doctors_Database-ALTER-doctors-WHERE-name-Parker-company-STUDIOLINK\n\n\n";

		cout << "INSERT row Statement:\n";
		cout << "DB_name-INSERT-INTO-tbl_name-VALS-[list_matching_the_table_columns]\n";
		cout << "Example:\n";
		cout << "Doctors_Database-INSERT-INTO-security_staff-VALS-Steve-police-YES\n\n\n";

		cout << "CREATE-DB Statement:\n";
		cout << "CREATE-DB-db_name-number_of_tables-[list_of_-separated_table_definitions]\n";
		cout << "Example:\n";
		cout << "CREATE-DB-vet_db-3-staff-3-name-age-gender-animals-5-species-date_of_birth-name-owner-sickness-suppliers-3-name-type-day\n\n\n";

		cout << "ADD table statement:\n";
		cout << "DB_name-ADD-tbl_name-number_of_columns-[list_of_column_names]\n";
		cout << "Example:\n";
		cout << "Doctors_Database-ADD-security_staff-3-name-role-present\n\n\n";

		cout << "DETELE an entry Statement:\n";
		cout << "DB_name-DELETE-ENTRY-tbl_name-column_name-string_matching_the_column\n";
		cout << "Example:\n";
		cout << "Doctors_Database-DELETE_ENTRY-doctors-name-Parker\n\n\n";

		cout << "DELETE TABLE Statement:\n";
		cout << "DB_name-DELETE-TABLE-tbl_name\n";
		cout << "Example:\n";
		cout << "Doctors_Database-DELETE-TABLE-security_staff\n\n\n";

		cout << "DELETE DB Statement:\n";
		cout << "DELETE-db_name\n\n\n";

		cout << "SHOW_ALL tables Statement:\n";
		cout << "DB_name-SHOW_ALL\n\n\n";

		cout << "SHOW_ALL columns Statement:\n";
		cout << "DB_name-tbl_name-SHOW_ALL\n\n\n";

		cout << "SHOW_ALL databases Statement:\n";
		cout << "SHOW_ALL\n";

		return;
	}
	if (cmd[0] == "SHOW_ALL") {
		cout << "Outputting database names...\n";
		for (size_t i = 0; i < databases.size(); ++i) {
			cout << databases[i].db_name << "\n";
		}
		return;
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
			cout << "ERROR: A database with the supplied name does not exist.\n";
			return;
		}
	}
	//Code for creating a database, BUT NOT for creating a table, goes here.
	else {
		found = false;
		if (cmd.size() < 4) {
			cout << "ERROR: Command to create a database is too short. Minimum size: 4\n";
			return;
		}
		string db_name = cmd[2], tbl_name = "";
		vector<Table> vec_tbls = {};
		vector<string> columns = {};
		if (stoi(cmd[3]) > 0) {

			int tbls = stoi(cmd[3]), pos_counter = 4, cols = 0;
			if (cmd.size() < ((3 * tbls) + 4)) {
				cout << "ERROR: Command to create a database is too short. Minimum required size: " << ((3 * tbls) + 4) << "\n";
				return;
			}
			for (int i = 0; i < tbls; ++i) {
				tbl_name = cmd[pos_counter];
				++pos_counter;
				//Get the number of columns in this table as written in SQL command.
				cols = stoi(cmd[pos_counter]);
				for (int j = 0; j < cols; ++j) {
					++pos_counter;
					columns.push_back(cmd[pos_counter]);
				}
				//After reading the rows, before continuing to next table:
				Table tbl_from_command(tbl_name, columns);
				vec_tbls.push_back(tbl_from_command);
				columns = {};
				++pos_counter;
			}
			Database db(db_name, vec_tbls);
			databases.push_back(db);
			return;
		}
		else {
			Database db(db_name);
			databases.push_back(db);
			return;
		}
	}
	//Add a table to an existing database.
	if (cmd.size() > 1) {


		if (cmd[1] == "ADD") {
			if (cmd.size() < 5) {
				cout << "ERROR: Command to add a table to a database is too short. Minimum size: 5\n";
				return;
			}
			size_t pos_counter = 2;
			string tbl_name = cmd[pos_counter];
			++pos_counter;
			int cols = stoi(cmd[pos_counter]);
			if (cmd.size() != (cols + 4)) {
				cout << "ERROR: Command to add a table to a database is of invalid length. Required length: " << cols + 4 << "\n";
				return;
			}
			++pos_counter;
			vector<string> col_names = {};
			for (size_t i = 0; i < cols; ++i) {
				col_names.push_back(cmd[pos_counter]);
				++pos_counter;
			}
			Table tbl(tbl_name, col_names);
			databases[db_pos].AddTable(tbl);
			return;
		}
		found = false;
		if (cmd[1] == "SHOW_ALL") {
			if (databases[db_pos].tables.size() == 0) {
				cout << "The database is empty.\n";
				return;
			}
			else {
				cout << "Outputting tables names...\n";
				for (size_t i = 0; i < databases[db_pos].tables.size(); ++i) {
					cout << databases[db_pos].tables[i].table_name << "\n";
				}
				return;
			}
		}
		if (cmd.size() > 2 && cmd[2] == "SHOW_ALL") {
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables.size(); ++i) {
				if (cmd[1] == databases[db_pos].tables[i].table_name) {
					cout << "Outputting table column names...\n";
					for (size_t j = 0; j < databases[db_pos].tables[i].T[0].size(); ++j) {
						cout << databases[db_pos].tables[i].T[0][j] << "\n";
					}
					return;
				}
			}
			cout << "ERROR: A table could not be located within this database.\n";
			return;


		}
		//DB_NAME-DELETE-TABLE-TBL_NAME
		if (cmd.size() > 2 && cmd[1] == "DELETE" && cmd[2] == "TABLE") {
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables.size(); ++i) {
				if (cmd[3] == databases[db_pos].tables[i].table_name) {
					found = true;
					databases[db_pos].tables.erase(databases[db_pos].tables.begin() + i);
					cout << "SUCCESS: Deleted the table from the database.\n";
					return;
				}
			}
			if (!found) {
				cout << "ERROR: A table could not be located within this database.\n";
				return;
			}
		}
		//DELETE-DB_NAME
		if (cmd[1] == "DELETE" && cmd.size() < 3) {
			databases.erase(databases.begin() + db_pos);
			cout << "SUCCESS: Deleted the database from the systen.\n";
			return;
		}
		found = false;
		//Insert an entry row into a table of an existing database.
		if (cmd[1] == "INSERT") {
			if (cmd.size() < 6) {
				cout << "ERROR: Command to insert an entry row into a table is too short. Minimum size: 6\n";
				return;
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
				cout << "ERROR: A table with the supplied name does not exist within this database.\n";
				return;
			}

			pos_counter += 2;
			size_t req_size = databases[db_pos].tables[tbl_pos].T[0].size() - 1;
			if ((cmd.size() - 5) != req_size) {
				cout << "ERROR: Command to insert an entry row into a table is of invalid size. Required size: " << (databases[db_pos].tables[tbl_pos].T[0].size() - 1) + 5 << "\n";
				return;
			}
			vector<string> input = {};
			for (int i = 0; i < req_size; ++i) {
				input.push_back(cmd[pos_counter]);
				++pos_counter;
			}
			databases[db_pos].tables[tbl_pos].Insert(input);
			return;
		}
		if (cmd[1] == "DELETE_ENTRY") {
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables.size(); ++i) {
				if (databases[db_pos].tables[i].table_name == cmd[2]) {
					found = true;
					tbl_pos = i;
					break;
				}
			}
			if (!found) {
				cout << "ERROR: A table with the supplied name does not exist within this database.\n";
				return;
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
				cout << "ERROR: A column with the name " << cmd[4] << " does not exist within this table.\n";
				return;
			}
			found = false;
			for (size_t i = 0; i < databases[db_pos].tables[tbl_pos].T.size(); ++i) {
				if (databases[db_pos].tables[tbl_pos].T[i][col_pos] == cmd[4]) {
					found = true;
					databases[db_pos].tables[tbl_pos].T.erase(databases[db_pos].tables[tbl_pos].T.begin() + i);
					cout << "SUCCESS: Deleted an entry from the table in this database.\n";
				}
			}
			if (!found) {
				cout << "ERROR: An entry with the specified value for this particular attribute does not exist within this table.\n";
				return;
			}
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
				cout << "ERROR: A table with the supplied name does not exist within this database.\n";
				return;
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
				cout << "ERROR: A column with the name " << cmd[4] << " does not exist within this table.\n";
				return;
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
				cout << "ERROR: An entry with the specified value for this particular attribute does not exist within this table.\n";
				return;
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
				cout << "ERROR: A column with the name " << cmd[6] << " does not exist within this table.\n";
				return;
			}
			if (cmd[7].size() <= 16) {
				for (size_t i = 0; i < row_positions.size(); ++i) {
					databases[db_pos].tables[tbl_pos].Alter(row_positions[i], col_pos, cmd[7]);
					cout << "SUCCESS: Alteration of table " << cmd[2] << " within database " << cmd[0] << " has been successful.\n";
				}
				return;
			}
			else {
				cout << "ERROR: The data item you tried to alter one of the tables' entries' attribute with is too long. Max length: 16\n";
				return;
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
				cout << "ERROR: A table with the supplied name does not exist within this database.\n";
				return;
			}
			if (databases[db_pos].tables[tbl_pos].T.size() == 1) {
				cout << "ERROR: The table you are trying to select items from is empty.\n";
				return;
			}
			found = false;
			vector<size_t> vecPOScol = {}, vecPOSrow = {};
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
							cout << "ERROR: A column with the name " << cmd[i] << " does not exist within this table.\n";
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
			for (size_t i = 0; i < conds.size(); ++i) {
				for (size_t j = 0; j < databases[db_pos].tables[tbl_pos].T[0].size(); ++j) {
					//cout << "i = " << i << ", j = " << j << ", conds.size() = " << conds.size() << ", T[0].size() = "
						//<< databases[db_pos].tables[tbl_pos].T[0].size() << "\n";
					//cout << "db_pos = " << db_pos << ", databases.size() = " << databases.size() <<
						//", tbl_pos = " << tbl_pos << ", tables.size() = " << databases[db_pos].tables.size() << "\n";
					//cout << "conds[i].size() = " << conds[i].size() << "\n";
					if (databases[db_pos].tables[tbl_pos].T[0][j] == conds[i][0]) {
						pos = j;
						cond_row_pos.push_back(j);
						break;
					}
					if (j == databases[db_pos].tables[tbl_pos].T[0].size() - 1) {
						cout << "ERROR: A column with the name " << conds[i][0] << " does not exist within this table.\n";
						break;
					}
				}
				if (conds[i][1] == ">" || conds[i][1] == "<") {
					//cout << "CALLING ContainsOnlyDigits on: " << databases[db_pos].tables[tbl_pos].T[1][pos] << " ... Result: "
						//<< ContainsOnlyDigits(databases[db_pos].tables[tbl_pos].T[1][pos]) << "\n";
					if (!(ContainsOnlyDigits(databases[db_pos].tables[tbl_pos].T[1][pos]))) {
						cout << "ERROR: Cannot use comparison operators on non-numeric items. Invalid condition: "
							<< conds[i][0] << " " << conds[i][1] << " " << conds[i][2] << "\n";
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
						//cout << "We can check against this condition.\n";
						//cout << "CALLING CheckCondition with " << databases[db_pos].tables[tbl_pos].T[i][cond_row_pos[j]] << ", " << conds[j][1] << ", " << conds[j][2] << "\n";
						//cout << "RESULT: " << CheckCondition(databases[db_pos].tables[tbl_pos].T[i][cond_row_pos[j]], conds[j][1], conds[j][2]) << "\n";
						check1 = CheckCondition(databases[db_pos].tables[tbl_pos].T[i][cond_row_pos[j]], conds[j][1], conds[j][2]);
						if (check1 == false) {
							//cout << "Fill becomes FALSE.\n";
							fill = false;
						}
					}
				}
				if (fill == true) {
					//cout << "Pushing row index: " << i << "\n";
					vecPOSrow.push_back(i);
				}
				else {
					fill = true;
				}
			}
			/* Uncomment to disable condition functionality.
			for (size_t i = 0; i < databases[db_pos].tables[tbl_pos].T.size(); ++i) {
				vecPOSrow.push_back(i);
			}
			*/
			databases[db_pos].tables[tbl_pos].Print(vecPOScol, vecPOSrow);
			vecPOSrow = {};
			vecPOScol = {};
			invalid_conds = {};
			cond_row_pos = {};
			conds = {};
			dummy_vec = {};
		}
	}
}

int main() {
	//CURRENT STATE: Loads but glitchy!! Still not finished!
	//Load the database from its saved state.
	
	string line, db_name = "", tbl_name = "";
	int ID_state = 0;
	bool db_nameb = false, tbl_nameb = false;
	vector<vector<string>> rows = {};
	vector<string> row_entries = {};
	vector<Table> tbls = {};
	int line_counter = 0;
	cout << "System loading...\n";
	ifstream myfile1("SavedDatabases.txt");
	if (myfile1.is_open())
	{
		while (getline(myfile1, line))
		{
			++line_counter;
			if (db_nameb) {
				db_name = line;
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
				//cout << "Seen a #. rows.size() = " << rows.size() << "\n";
				if (!rows.size() == 0) {
					Table tbl(tbl_name, rows[0]);
					//cout << "After creating table, tbl.T.size() = " << tbl.T.size() << ". db is " << db_name << ". tbl is " << tbl_name << "\n";
					tbl.T[0].erase(tbl.T[0].begin());
					for (size_t i = 1; i < rows.size(); ++i) {
						tbl.T.push_back(rows[i]);
					}
					tbl.number_of_entries = tbl.T.size();
					if (tbl.T.size() == 1) {
						tbl.primary_key = 0;
					}
					else {
						//cout << "tbl.T.size() = " << tbl.T.size() << "\n";
						tbl.primary_key = stoull(tbl.T[tbl.T.size() - 1][0]);
					}
					tbls.push_back(tbl);
					rows = {};
				}
				tbl_nameb = true;
			}
			else if (line == "TBL_END") {
				//cout << "Seen a TBL_END. rows.size() = " << rows.size() << "\n";
				Table tbl(tbl_name, rows[0]);
				//cout << "After creating FINAL table, tbl.T.size() = " << tbl.T.size() << ". db is " << db_name << ". tbl is " << tbl_name << "\n";
				tbl.T[0].erase(tbl.T[0].begin());
				for (size_t i = 1; i < rows.size(); ++i) {
					tbl.T.push_back(rows[i]);
				}
				tbl.number_of_entries = tbl.T.size();
				if (tbl.T.size() == 1) {
					tbl.primary_key = 0;
				}
				else {
					tbl.primary_key = stoull(tbl.T[tbl.T.size() - 1][0]);
				}
				tbls.push_back(tbl);
				rows = {};

				Database in_db(db_name, tbls);
				databases.push_back(in_db);
				tbls = {};
			}
			//The end of reading.
			else if (line == "SYS_END") {
				cout << "System loaded successfully.\n";
				break;
			}
			else {
				rows.push_back(SeparateAllWords(line));
			}
		}
		myfile1.close();
	}
	
	//Begin input of SQL commands.
	string input_command = "";
	cout << "Welcome to the database management system. For a list of commands, type 'ls'. To exit the system, type 'QUIT'.\n";
	while (cin >> input_command) {
		if (input_command == "QUIT") { cout << "Terminating input of commands...\n"; break; }
		else {
			SQL_Command_Interpreter(input_command);
		}
	}
	//Save the database.
	cout << "System saving...\n";
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
	cout << "System exiting...\n";
	return 0;
} 
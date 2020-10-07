#include <string>
#include <cmath>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
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
		operand1 = stod(operand1);
		operand2 = stod(operand2);
		if (Operator == "<") {
			if (operand1 >= operand2) {
				holds = false;
			}
		}
		else if (Operator == "=") {
			if (operand1 != operand2) {
				holds = false;
			}
		}
		else if (Operator == ">") {
			if (operand1 <= operand2) {
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
	//Can just call the constructors with parameters fetched from the command to create dbs/tables
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

int main() {
	//Create a sample database.
	vector<string> vec = { "name", "color", "company" };
	vector<string> values1 = { "John", "pink", "LTD_comp" }, values2 = { "Parker", "black", "SDF_regex" }, values3 = { "Carper", "green", "SDF_regex" };
	string name = "doctors";

	Table T1(name, vec);

	T1.ShowAttributes();

	T1.Insert(values1);
	T1.Insert(values2);
	T1.Insert(values3);
	
	unsigned index = 1;

	string dbname = "Doctors_Database";
	vector<Table> vec1 = { T1 };

	Database DB1(dbname, vec1); databases.push_back(DB1);
	DB1.AddTable(T1);
	cout << "Number of entities in the database: " << DB1.NumberOfTables() << ". Removing one next...\n";
	DB1.RemoveTable(name);
	cout << "Number of entities in the database: " << DB1.NumberOfTables() << ". Removing a non-existent table next...\n";
	string non_existent_name = "paradise";
	DB1.RemoveTable(non_existent_name);

	vector<unsigned> column_indices = { 0, 1, 2, 3 }, row_indices = { 0, 1, 2, 3 };
	T1.Print(column_indices, row_indices);
	index = 1;
	//T1.Delete(index);
	//cout << "After deleting row with primary key 1, the number of entries in the table is: " << T1.number_of_entries << " = " << (T1.T).size() - 1 << "\n";
	string new_entry = "purple";
	size_t row_i = 1, column_i = 2;
	T1.Alter(row_i, column_i, new_entry);
	cout << "After altering [1][2] to 'purple', the table looks like this:\n";
	T1.Print(column_indices, row_indices);
	

	//Begin input of SQL commands.
	string input_command = "";
	while (cin >> input_command) {
		if (input_command == "QUIT") { cout << "Terminating input of commands...\n"; break; }
		else {
			SQL_Command_Interpreter(input_command);
		}
	}
	databases[0].tables[1].ShowAttributes();
	cout << "Exiting the system...\n";
	return 0;
} 
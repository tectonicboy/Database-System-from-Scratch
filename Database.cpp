#include <string>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>

using namespace std;

class Table {
public:
	string table_name = "";
	vector<vector<string>> T = {};
	unsigned number_of_entries = 0;
	unsigned primary_key = 0LL;

	Table(string& s, vector<string>& v) : table_name(s) {
		v.insert(v.begin(), 1, "ID");
		T.push_back(v);
	}

	//Insert an entry to the table with provided values [INSERT INTO name VALUES vals]
	void Insert(vector<string>& v) {
		if (v.size() == T[0].size() - 1) {
			v.insert(v.begin(), 1, to_string(primary_key));
			++primary_key;
			++number_of_entries;
			T.push_back(v);
		}
		else if (v.size() < T[0].size() - 1) {
			cout << "ERROR: Too few arguments supplied (" << v.size() << "). Please supply " << (T[0].size() - 1) << " arguments in order to add a row to this table.\n";
		}
		else if (v.size() > T[0].size() - 1) {
			cout << "ERROR: Too many arguments supplied (" << v.size() << "). Please supply " << (T[0].size() - 1) << " arguments in order to add a row to this table.\n";
		}
	}


	//On delete, number_of_entries decrements, but primary_key does not.
	void Delete(unsigned& key) {
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
			cout << "SUCCESS: Erasing the row at index: " << position_index << " (primary key: " << key << ")\n";
			T.erase(T.begin() + position_index);
			--number_of_entries;
		}
		else {
			cout << "ERROR: An entry inside the table with the provided primary key could not be located.\n";
		}
	}

	void Alter(size_t& row_index, size_t& column_index, string&s) {
		unsigned int max_len = 16;
		if (T.size() > row_index && T[row_index].size() > column_index && s.size() <= 16) {
			cout << "SUCCESS: Setting T[" << row_index << "][" << column_index << "] to " << s << "\n";
			T[row_index][column_index] = s;
		}
		else if (s.size() >= max_len) {
			cout << "ERROR: The element you tried entering is too long. Max length of an entry: 16\n";
		}
		else if (T.size() <= row_index || T[row_index].size() <= column_index) {
			cout << "ERROR: The position at which you attempted to modify an element could not be located in this table.\n";
		}

	}
	//Print elements of a table [SELECT-[* || string_list]-FROM-table_name-WHERE-[condition(s)]].
	void Print(vector<unsigned> &VC, vector<unsigned> &VR) {
		size_t spaces = 0, half_spaces = 0;
		bool odd = false;
		for (unsigned i = 0; i < VR.size(); ++i) {
			for (unsigned j = 0; j < VC.size(); ++j) {
				if (T.size() > VR[i] && T[VR[i]].size() > VC[j]) {
					spaces = 16 - T[VR[i]][VC[j]].size();
					if ((spaces % 2) == 1) { odd = true; --spaces; }
					half_spaces = spaces / 2;
					while (half_spaces > 0) {
						cout << " ";
						--half_spaces;
					}
					cout << T[VR[i]][VC[j]];
					half_spaces = spaces / 2;
					while (half_spaces > 0) {
						cout << " ";
						--half_spaces;
					}
					if (odd) { cout << " "; }
					cout << "|";
					odd = false;
				}
				else (cout << "ERROR: Element to print not located.\n");

			}
			cout << "\n";
		}
	}
	//Show the column names of the table (the core row)
	void ShowAttributes(void) {
		cout << "Attributes of table '" << table_name << "': ";
		for (size_t i = 0; i < T[0].size() - 1; ++i) {
			cout << T[0][i] << ", ";
		}
		cout << T[0][T[0].size() - 1] << "\n";
	}
};
class Database {
public:
	string db_name = "";
	vector<Table> tables = {};
	
	int NumberOfTables(void) const {
		return (int)tables.size();
	}

	Database(string &s, vector<Table> &v) : db_name(s), tables(v) {};
	Database(string &s) : db_name(s) {
		tables = {};
	};

	void AddTable(Table& T) { tables.push_back(T); }

	void RemoveTable(string& s) {
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
			cout << "SUCCESS: Removing table '" << tables[position].table_name << "'\n";
			tables.erase(tables.begin() + position);
		}
		else {
			cout << "ERROR: A table with the supplied name could not be located.\n";
		}
	}
};


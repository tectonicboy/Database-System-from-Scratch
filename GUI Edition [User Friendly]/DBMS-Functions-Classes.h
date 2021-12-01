#include "cApp.h"

extern cMain** ptr_obj;

#include <string>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <ctime>
#include <unistd.h>

using namespace std;

class Table {
public:
	string table_name = "";
	vector<vector<string>> T = {};
	unsigned number_of_entries = 0;
	unsigned primary_key = 0LL;

	Table(string& s, vector<string>& v);
	//Insert an entry to the table with provided values [INSERT INTO name VALUES vals]
	void Insert(vector<string>& v);

	//On delete, number_of_entries decrements, but primary_key does not.
	void Delete(unsigned& key);

	void Alter(size_t& row_index, size_t& column_index, string& s);

	//Print elements of a table [SELECT-[* || string_list]-FROM-table_name-WHERE-[condition(s)]].
	void Print(vector<unsigned>& VC, vector<unsigned>& VR);

	//Show the column names of the table (the core row)
	void ShowAttributes(void);
};
class Database {
public:
	string db_name = "";
	vector<Table> tables = {};

	int NumberOfTables(void) const;

	Database(string& s, vector<Table>& v);
	Database(string& s);

	void AddTable(Table& T);

	void RemoveTable(string& s);
};
//***************************************************************************************************
//********************************* MAIN DBMS FUNCTIONS FOLLOW **************************************
//***************************************************************************************************

using namespace std;

extern vector<Database> databases;

vector<string> SeparateAllWords(string s);

bool ContainsOnlyDigits(string s);

bool CheckCondition(string operand1, string Operator, string operand2);

vector<string> SeparateCondition(string s);

bool SQL_Command_Interpreter(string& command);

void SaveSystem(bool out_msg = true);

void LoadSystem(bool out_msg = true);

//Open file SERVER_SQL_COMMANDS.txt, execute all commands on it, and clear it.
extern bool DB_UPDATE;

void UpdateDatabase(string& filename);

//*********************************************************************************************************************
//************************************************* END OF MAIN DBMS FUNCTIONS ****************************************
//*********************************************************************************************************************

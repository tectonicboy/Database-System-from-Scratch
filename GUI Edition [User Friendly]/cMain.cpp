#include "DBMS Functions + Classes.h"
#include "cMain.h"
#include <thread>

cMain** ptr_obj;
extern bool DB_UPDATE;

#define DISP_SIZE wxGetDisplaySize()
#define DISP_X wxGetDisplaySize().x
#define DISP_Y wxGetDisplaySize().y

wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(10001, cMain::OnButtonClicked)

	EVT_BUTTON(10002, cMain::OnAboutBtnClicked)
	EVT_BUTTON(10003, cMain::OnSQLBtnClicked)
	EVT_BUTTON(10004, cMain::OnSchemaBtnClicked)
	EVT_BUTTON(10005, cMain::OnHistoryBtnClicked)
	EVT_BUTTON(10006, cMain::OnHelpBtnClicked)
	EVT_BUTTON(10007, cMain::OnContactBtnClicked)
	EVT_BUTTON(10008, cMain::OnLegalBtnClicked)

	EVT_BUTTON(10010, cMain::OnSchBtnGOClicked)
wxEND_EVENT_TABLE()

using namespace std;

//*********** COLOR DEFINITIONS **********
wxColour
hot_pink( 245, 2, 87 ),
magenta( 143, 82, 209 ),
light_lime( 106, 250, 92 ),
golden( 252, 255, 87 ),
dark_purple( 46, 0, 37 ),
light_green( 220, 255, 204 ),
blue( 0, 200, 255 ),
green_gray( 89, 99, 89 ),
pale_red( 173, 111, 111 ),
pale_green( 86, 163, 89 ),
solid_green( 107, 209, 110 ),
grim_purple( 189, 2, 123 ),
solid_blue( 46, 15, 186 ),
dark_blue(0, 2, 64),
bright_blue( 25, 224, 218 ),
lightdark_purple( 122, 0, 100 ),
grim_green( 6, 43, 31 ),
black_purple( 43, 6, 41 );
//****************************************


//********* FONT DEFINITIONS *************
wxFont
font_1(wxFontInfo(12).Bold()),
font_2(wxFontInfo(20).Bold()),
font_3(wxFontInfo(12)),
font_4(wxFontInfo(11).Bold()),
font_5(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_EXTRALIGHT);
//****************************************

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "Custom Database Management System", wxPoint(0, 0), DISP_SIZE)
{
	this->SetBackgroundColour(grim_green);

	//******************************** SQL SECTION BEGIN ***********************************************************************
	m_btn1 = new wxButton(this, 10001, "GO", wxPoint(10, 60), wxSize(55, 55));
	m_txt1 = new wxTextCtrl(this, wxID_ANY, "Enter SQL command...", wxPoint(10, 120), wxSize(DISP_X - 20, 26));
	m_main_output = new wxListBox(this, wxID_ANY, wxPoint(10, 160), wxSize(DISP_X - 20, DISP_Y - 400));
	m_err_output = new wxListBox(this, wxID_ANY, wxPoint(10 , 160+(DISP_Y - 400)), wxSize(DISP_X-20, 200));

	m_main_output->SetBackgroundColour(dark_blue);
	m_main_output->SetForegroundColour(bright_blue);
	m_main_output->SetFont(font_5);

	m_err_output->SetBackgroundColour(black_purple);
	m_err_output->SetForegroundColour(hot_pink);
	m_err_output->SetFont(font_1);

	m_btn1->SetBackgroundColour(golden);
	m_btn1->SetForegroundColour(solid_blue);
	m_btn1->SetFont(font_2);

	m_txt1->SetBackgroundColour(dark_blue);
	m_txt1->SetForegroundColour(bright_blue);
	m_txt1->SetFont(font_3);
	//******************************** SQL SECTION END *************************************************************************

	//******************************** MENU BUTTONS BEGIN **********************************************************************
	vector<string> menu_labels = { "About", "SQL", "Schema+", "History", "Help", "Contact", "Legal"};
	for (size_t i = 0; i < 7; ++i) {
		wxButton* ptr = new wxButton(this, (10002 + i), menu_labels[i], wxPoint((i*78)+10, 0), wxSize(80, 40));
		ptr->SetBackgroundColour(black_purple);
		ptr->SetForegroundColour(hot_pink);
		ptr->SetFont(font_4);
		m_menu_btns.push_back(ptr);
	}
	//******************************** MENU BUTTONS END ***********************************************************************

	//********************************* ABOUT SECTION BEGIN *************************************************************************
	m_about_txt = new wxListBox(this, wxID_ANY, wxPoint(10, 50), wxSize(DISP_X - 30, DISP_Y - 90));
	m_about_txt->SetBackgroundColour(black_purple);
	m_about_txt->SetForegroundColour(bright_blue);
	m_about_txt->SetFont(font_1);
	vector<string> txt_strings = {
		"", "",
		"						Welcome onboard the Custom Relational Database Management System!", "", "", "",
		"OVERVIEW",
		"This is a free time project aimed at improving my programming skills and knowledge.", "",
		"It is a relational database management system built completely from scratch in C++.", "",
		"It comes in two editions: the command-line tool and the GUI-enriched tool (this edition).", "",
		"A custom SQL language syntax has been created, and an interpreter programmed for it. You can see the list of available SQL commands in the Help menu.", "",
		"The whole system, both editions, was created by Kevin Stefanov, whose contact details can be found in the Contact menu.", "",
		"The end goal of the project is the enable a personally-built website to use this database management system on a personal server, instead of MySQL, to manage website data.", "",
		"PROGRAMMING",
		"- Two database management classes, a global function standing as the interpreter for the custom SQL syntax, and a few additional functions make up the DBMS.", "",
		"- Primary keys are allocated automatically in a +1 manner. Furthermore, data types are not supported, meaning all table data is in the form of text, and it is",
		"  up to you, the user, to make sure that logically correct data types are used for all data throughout the operation and use of the system.", "",
		"- Significant error-detection is built into the system, letting the user know when a command has been wrongly written, or cannot be executed for various reasons.", "",
		"- A Save/Load subsystem, which outputs all vital data about all databases on the system to a text file, and then reads it back to load, has been programmed.", "",
		"- The Graphical User Interface has been written using wxWidgets in C++.",
	};
	for (size_t i = 0; i < txt_strings.size(); ++i) {
		if (txt_strings[i] == "PROGRAMMING") {
			for (size_t j = 0; j < 5; ++j) { m_about_txt->AppendString(""); }
			m_about_txt->AppendString(txt_strings[i]);
		}
		else{ m_about_txt->AppendString(txt_strings[i]); }
	}
	//************************************** ABOUT SECTION END ***********************************************************************

	//************************************** HELP SECTION BEGIN **********************************************************************
	m_help_txt = new wxListBox(this, wxID_ANY, wxPoint(10, 50), wxSize(DISP_X - 30, DISP_Y - 90));
	m_help_txt->SetBackgroundColour(black_purple);
	m_help_txt->SetForegroundColour(golden);
	m_help_txt->SetFont(font_1);
	txt_strings = {
		"Using the SQL Section", "", "",
		"- First, type in any valid command conforming to the custom SQL language syntax in the top text box.", "",
		"- Hit GO.", "",
		"- You should then see an output message in the bottom text box, describing the output of your command.", "",
		"- If it succeeds, it will let you know so, and provide basic information on what has been accomplished with it.", "",
		"- If it failed, it will tell you what went wrong and the reason why the command could not be executed.", "",
		"Using the Schema+ Section", "", "",
		"- You can use this section to both create database(s) from a schema and/or also insert data into the newely created databases at the same time.", "",
		"  by using multiple SQL commands at once.", "",
		"- After inputting the desired set of SQL commands, hit the GO button.", "",
		"- As with the SQL section, error/success messages will appear in the text box.", "",
		"- Note that any non-system output generated from your commands will appear on the main output window in the SQL section."
	};
	for (size_t i = 0; i < txt_strings.size(); ++i) {
		if (txt_strings[i] == "Using the Schema+ Section") {
			for (size_t j = 0; j < 5; ++j) { m_help_txt->AppendString(""); }
			m_help_txt->AppendString(txt_strings[i]);
		}
		else { m_help_txt->AppendString(txt_strings[i]); }
	}
	//************************************** HELP SECTION END ************************************************************************

	//************************************** CONTACT SECTION BEGIN *******************************************************************
	m_contact_txt = new wxListBox(this, wxID_ANY, wxPoint(10, 50), wxSize(DISP_X - 30, DISP_Y - 90));
	m_contact_txt->SetBackgroundColour(black_purple);
	m_contact_txt->SetForegroundColour(light_lime);
	m_contact_txt->SetFont(font_1);
	txt_strings = {
		"Contact details of system creator, Kevin Stefanov:", "", "",
		"- e-mail address: kevinstefanov15@gmail.com", "",
		"- phone number: +447507275797", ""
	};
	for (size_t i = 0; i < txt_strings.size(); ++i) {
		m_contact_txt->AppendString(txt_strings[i]);
	}

	//************************************** CONTACT SECTION END **********************************************************************

	//*************************************** LEGAL SECTION BEGIN *********************************************************************
	m_legal_txt = new wxListBox(this, wxID_ANY, wxPoint(10, 50), wxSize(DISP_X - 30, DISP_Y - 90));
	m_legal_txt->SetBackgroundColour(black_purple);
	m_legal_txt->SetForegroundColour(light_lime);
	m_legal_txt->SetFont(font_1);
	txt_strings = {
		"LEGAL INFORMATION", "", "",
		"This system is freely available and usable, as long as proper credits are listed.", ""
	};
	for (size_t i = 0; i < txt_strings.size(); ++i) {
		m_legal_txt->AppendString(txt_strings[i]);
	}
	//*************************************** LEGAL SECTION END ***********************************************************************

	//*************************************** HISTORY SECTION BEGIN *******************************************************************
	m_history_txt = new wxListBox(this, wxID_ANY, wxPoint(10, 50), wxSize(DISP_X - 50, DISP_Y - 90));
	m_history_txt->SetBackgroundColour(black_purple);
	m_history_txt->SetForegroundColour(light_lime);
	m_history_txt->SetFont(font_5);
	//*************************************** HISTORY SECTION END *********************************************************************

	//*************************************** SCHEMA+ SECTION BEGIN *******************************************************************
	m_sch_code = new wxTextCtrl(this, wxID_ANY, "Enter SQL commands...", wxPoint(10,110), wxSize(DISP_X - 25, DISP_Y - 360), wxTE_MULTILINE);
	m_sch_code->SetBackgroundColour(dark_blue);
	m_sch_code->SetForegroundColour(bright_blue);
	m_sch_code->SetFont(font_3);

	m_sch_btn = new wxButton(this, 10010, "GO", wxPoint(10, 50), wxSize(55, 55));
	m_sch_btn->SetBackgroundColour(golden);
	m_sch_btn->SetForegroundColour(solid_blue);
	m_sch_btn->SetFont(font_2);
	//*************************************** SCHEMA+ SECTION END *********************************************************************

	m_btn1->Hide();
	m_txt1->Hide();
	m_err_output->Hide();
	m_main_output->Hide();
	m_help_txt->Hide();
	m_contact_txt->Hide();
	m_legal_txt->Hide();
	m_history_txt->Hide();
	m_sch_code->Hide();
	m_sch_btn->Hide();
}

cMain::~cMain() {
	SaveSystem();
}
//Every 10 seconds, look at SERVER_SQL_COMMANDS.txt, execute commands on it and clear it, in a separate thread.
std::string sql_filename = "SERVER_SQL_COMMANDS.txt";

//Note the use of std::ref(). When creating a thread with a function that takes an argument by reference with &, we
//have to wrap said argument in std::ref() when using it to call the function during thread creation, otherwise it wont work, only by value.
std::thread t1(UpdateDatabase, std::ref(sql_filename));

//SQL Command Entry Button.
void cMain::OnButtonClicked(wxCommandEvent& evt) {
	wxString input = m_txt1->GetValue();
	string real_input = input.ToStdString();
	m_history_txt->AppendString(real_input);
	m_txt1->Clear();
	if (real_input == "") {
		evt.Skip();
		return;
	}
	if (real_input == "EXIT") {
		//DB_UPDATE = false;
		//~t1;
		//t1.join();
		delete this;
	}
	else if (real_input == "LOAD") {LoadSystem();}
	else if (real_input == "SAVE") {SaveSystem();}
	else {
		SQL_Command_Interpreter(real_input);
		m_main_output->AppendString("");
	}
	evt.Skip();
}

//Schema+ Entry Button.
void cMain::OnSchBtnGOClicked(wxCommandEvent& evt) {
	wxString sch_input = m_sch_code->GetValue();
	string command = "";
	for (size_t i = 0; i < sch_input.size(); ++i) {
		if (sch_input[i] != ';') {
			if (sch_input[i] != 10){command.push_back(sch_input[i]);}
		}
		else {
			m_history_txt->AppendString(command);
			if (command == "EXIT") {
				DB_UPDATE = false;
				t1.join();
				delete (*ptr_obj);
			}
			else if (command == "LOAD") { LoadSystem(); }
			else if (command == "SAVE") { SaveSystem();}
			else {SQL_Command_Interpreter(command);}
			command = "";
		}
	}
	evt.Skip();
}

void cMain::HideAll(void) {
	m_btn1->Hide();
	m_txt1->Hide();
	m_err_output->Hide();
	m_main_output->Hide();
	m_about_txt->Hide();
	m_help_txt->Hide();
	m_contact_txt->Hide();
	m_legal_txt->Hide();
	m_history_txt->Hide();
	m_sch_code->Hide();
	m_sch_btn->Hide();
}

void cMain::OnSQLBtnClicked(wxCommandEvent& evt) {
	HideAll();
	m_btn1->Show();
	m_txt1->Show();
	m_err_output->Show();
	m_main_output->Show();
	evt.Skip();
}

void cMain::OnSchemaBtnClicked(wxCommandEvent& evt) {
	HideAll();
	m_sch_code->Show();
	m_sch_btn->Show();
	m_err_output->Show();
	evt.Skip();
}

void cMain::OnHistoryBtnClicked(wxCommandEvent& evt) {
	HideAll();
	m_history_txt->Show();
	evt.Skip();
}

void cMain::OnAboutBtnClicked(wxCommandEvent& evt) {
	HideAll();
	m_about_txt->Show();
	evt.Skip();
}

void cMain::OnHelpBtnClicked(wxCommandEvent& evt) {
	HideAll();
	m_help_txt->Show();
	evt.Skip();
}

void cMain::OnContactBtnClicked(wxCommandEvent& evt) {
	HideAll();
	m_contact_txt->Show();
	evt.Skip();
}

void cMain::OnLegalBtnClicked(wxCommandEvent& evt) {
	HideAll();
	m_legal_txt->Show();
	evt.Skip();
}

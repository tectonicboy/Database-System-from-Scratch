#include "DBMS Functions + Classes.cpp"
#include "cMain.h"

cMain** ptr_obj;

#define DISP_SIZE wxGetDisplaySize()
#define DISP_X wxGetDisplaySize().x
#define DISP_Y wxGetDisplaySize().y

wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(10001, OnButtonClicked)
wxEND_EVENT_TABLE()

//*********** COLOR DEFINITIONS **********
wxColour
hot_pink({ 245, 2, 87 }),
magenta({ 143, 82, 209 }),
light_lime({ 106, 250, 92 }),
golden({ 252, 255, 87 }),
dark_purple({ 46, 0, 37 }),
light_green({ 220, 255, 204 }),
blue({ 0, 200, 255 }),
green_gray({ 89, 99, 89 }),
pale_red({ 173, 111, 111 }),
pale_green({ 86, 163, 89 }),
solid_green({ 107, 209, 110 }),
grim_purple({ 189, 2, 123 }),
solid_blue({ 46, 15, 186 });
//****************************************


//********* FONT DEFINITIONS *************
wxFont
font_1(wxFontInfo(14).Bold()),
font_2(wxFontInfo(20).Bold()),
font_3(wxFontInfo(12).Bold());
//****************************************

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "Custom Database Management System", wxPoint(0, 0), DISP_SIZE)
{
	m_btn1 = new wxButton(this, 10001, "GO", wxPoint(10, 60), wxSize(55, 55));
	m_txt1 = new wxTextCtrl(this, 10002, "Enter command...", wxPoint(10, 120), wxSize(DISP_X - 20, 26));
	m_main_output = new wxListBox(this, wxID_ANY, wxPoint(10, 160), wxSize(DISP_X - 20, DISP_Y - 400));
	m_err_output = new wxListBox(this, wxID_ANY, wxPoint(10 , 160+(DISP_Y - 400)), wxSize(DISP_X-20, 200));


	this->SetBackgroundColour(pale_green);
	//this->SetTransparent(195);

	m_main_output->SetBackgroundColour(solid_green);
	m_main_output->SetForegroundColour(solid_blue);
	m_main_output->SetFont(font_1);

	m_err_output->SetBackgroundColour(solid_green);
	m_err_output->SetForegroundColour(hot_pink);
	m_err_output->SetFont(font_1);

	m_btn1->SetBackgroundColour(solid_green);
	m_btn1->SetForegroundColour(solid_blue);
	m_btn1->SetFont(font_2);

	m_txt1->SetBackgroundColour(solid_green);
	m_txt1->SetForegroundColour(solid_blue);
	m_txt1->SetFont(font_3);

}

cMain::~cMain() {
}


void cMain::OnButtonClicked(wxCommandEvent& evt) {
	wxString input = m_txt1->GetValue();
	std::string real_input = input.ToStdString();
	if (real_input == "EXIT") {
		SaveSystem();
	}
	else if (real_input == "LOAD") {
		LoadSystem();
	}
	else {
		SQL_Command_Interpreter(real_input);
	}
	evt.Skip();
}





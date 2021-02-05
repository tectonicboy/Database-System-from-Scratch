#ifndef MAIN_HEADER
#define MAIN_HEADER
#include "wx/wx.h"

class cMain : public wxFrame
{
public:
	cMain();
	~cMain();
public:
	std::vector<wxButton*> m_menu_btns = {};


	//*********** SQL section *******************
	wxButton* m_btn1 = nullptr;
	wxTextCtrl* m_txt1 = nullptr;
	wxListBox* m_main_output = nullptr;
	wxListBox* m_err_output = nullptr;
	void OnSQLBtnClicked(wxCommandEvent& evt);

	//********** Schema+ section ****************
	wxTextCtrl* m_sch_code = nullptr;
	wxButton* m_sch_btn = nullptr;
	void OnSchemaBtnClicked(wxCommandEvent& evt);
	void OnSchBtnGOClicked(wxCommandEvent& evt);

	//********** History section ****************
	wxListBox* m_history_txt = nullptr;
	void OnHistoryBtnClicked(wxCommandEvent& evt);

	//*********** About section *****************
	wxListBox* m_about_txt = nullptr;
	void OnAboutBtnClicked(wxCommandEvent& evt);

	//********** Help section *******************
	wxListBox* m_help_txt = nullptr;
	void OnHelpBtnClicked(wxCommandEvent& evt);

	//********** Contact section ****************
	wxListBox* m_contact_txt = nullptr;
	void OnContactBtnClicked(wxCommandEvent& evt);

	//********** Legal section ******************
	wxListBox* m_legal_txt = nullptr;
	void OnLegalBtnClicked(wxCommandEvent& evt);

	void OnButtonClicked(wxCommandEvent& evt);
	void HideAll(void);
	wxDECLARE_EVENT_TABLE();
};

#endif





#ifndef MAIN_HEADER
#define MAIN_HEADER
#include "wx/wx.h"

class cMain : public wxFrame
{
public:
	cMain();
	~cMain();
public:
	wxButton* m_btn1 = nullptr;
	wxTextCtrl* m_txt1 = nullptr;
	wxListBox* m_main_output = nullptr;
	wxListBox* m_err_output = nullptr;

	void OnButtonClicked(wxCommandEvent& evt);
	wxDECLARE_EVENT_TABLE();


};

#endif





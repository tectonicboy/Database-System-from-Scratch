#ifndef APP_CLASS
#define APP_CLASS

#include "wx/wx.h"
#include "cMain.h"
class cApp : public wxApp
{
public:
	cApp();
	~cApp();
public:
	cMain* m_frame1 = nullptr;
public:
	virtual bool OnInit();
};


#endif


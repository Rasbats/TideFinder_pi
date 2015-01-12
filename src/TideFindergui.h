///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __TideFinderGUI_H__
#define __TideFinderGUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/statbox.h>
#include <wx/event.h>
#include <wx/listbox.h>
#include <wx/datectrl.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DlgDef
///////////////////////////////////////////////////////////////////////////////
class DlgDef : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText1511;
		wxStaticText* m_staticText32111112;
		wxTextCtrl* m_Speed_PS;
		wxStaticText* m_staticText33111111;
		wxListBox* m_listBox1; 
		wxStaticText* m_staticText8;
		wxStaticText* m_staticText32111111;
		wxChoice* m_Nship;
		wxStaticText* m_staticText3311111;
		wxButton* CAL_button;
		wxButton* PR_button;
		wxButton* NX_button;
		wxButton* GF_button;
		wxStaticText* m_staticText7;
		wxTextCtrl* m_Route;
		
		wxButton* m_Cursor;
		
		//wxTextCtrl* m_Lon1;
		wxStaticText* m_staticText1; 
		wxStaticText* m_staticText2;
		wxStaticText* m_staticText3;
		wxStaticText* m_staticText4;

		//wxChoice* m_choice1;
		//wxChoice* m_choice2;

		wxDatePickerCtrl* m_datePicker1;
		
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void PREvent( wxCommandEvent& event ) { event.Skip(); }
		virtual void NXEvent( wxCommandEvent& event ) { event.Skip(); }
		virtual void GFEvent( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCalendarShow( wxCommandEvent& event ) { event.Skip(); }
		
	public:
		
		wxTextCtrl* m_Lat1;
		
		
		wxBoxSizer* bSframe;

		DlgDef( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("TideFinder"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 438,210 ), long style = wxCAPTION|wxCLOSE_BOX|wxDIALOG_NO_PARENT|wxRESIZE_BORDER|wxSYSTEM_MENU ); 
		~DlgDef();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class CfgDlgDef
///////////////////////////////////////////////////////////////////////////////
class CfgDlgDef : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText17;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
	
	public:
		
		CfgDlgDef( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("TideFinder preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~CfgDlgDef();
	
};

#endif //__TideFinderGUI_H__

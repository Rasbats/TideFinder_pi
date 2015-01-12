///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "TideFindergui.h"

enum  // just in case this is needed in the future
{
      ID_TCWIN_NX,
      ID_TCWIN_PR
};


///////////////////////////////////////////////////////////////////////////

DlgDef::DlgDef( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	
	bSframe = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer* bSizer1311;
	bSizer1311 = new wxBoxSizer( wxVERTICAL );

	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("Use Right-Click in the location for tide prediction"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	
	wxFont *pTCFont2;
	pTCFont2 = wxTheFontList->FindOrCreateFont( 12, wxDEFAULT, wxNORMAL, wxFONTWEIGHT_NORMAL , FALSE,
      wxString( _T ( "Eurostile Extended" ) ) );
	
	m_staticText2->SetFont(*pTCFont2);
	wxColour c_blue = wxColour(220, 220, 220);
	m_staticText2->SetBackgroundColour(c_blue);
	
	bSizer1311->Add( m_staticText2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	// The date/time/zone
	m_staticText3 = new wxStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 ); //MyDateTime
	
    wxFont *pTCFont1;
	pTCFont1 = wxTheFontList->FindOrCreateFont( 12, wxDEFAULT, wxNORMAL, wxFONTWEIGHT_NORMAL , FALSE,
                                                wxString( _T ( "Eurostile Extended" ) ) );
	m_staticText3->SetFont(*pTCFont1);
	
	m_staticText3->Wrap( -1 );
	bSizer1311->Add( m_staticText3, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	wxFont *pTCFont3;
	pTCFont3 = wxTheFontList->FindOrCreateFont( 12, wxDEFAULT, wxNORMAL, wxFONTWEIGHT_NORMAL , FALSE,
      wxString( _T ( "Eurostile Extended" ) ) );

	m_listBox1 = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL,  wxLB_NEEDED_SB ); 
	m_listBox1->SetFont(*pTCFont3);
	m_listBox1->SetBackgroundColour( wxColour( 224, 224, 224 ) );

	bSizer1311->Add( m_listBox1, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	m_listBox1->Hide();


	bSframe->Add( bSizer1311, 0, 0, 5 );

	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer( wxHORIZONTAL );

	CAL_button = new wxButton( this, wxID_ANY, _("Select Date"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer71->Add( CAL_button, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	CAL_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgDef::OnCalendarShow ), NULL, this );

	PR_button = new wxButton( this, ID_TCWIN_PR, _("Prev"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer71->Add( PR_button, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	PR_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgDef::PREvent ), NULL, this );

    NX_button = new wxButton( this, ID_TCWIN_NX, _("Next"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer71->Add( NX_button, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );	
	
	NX_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgDef::NXEvent ), NULL, this );

	GF_button = new wxButton( this, ID_TCWIN_NX, _("Graph"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer71->Add( GF_button, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );	
	
	GF_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgDef::GFEvent ), NULL, this );
	
	CAL_button->Hide();
    PR_button->Hide();
	NX_button->Hide();
	GF_button->Hide();

	bSframe->Add( bSizer71, 1, wxALIGN_CENTER, 5 );
		
	this->SetSizer( bSframe );
	this->Layout();
	
	this->Centre( wxBOTH );

	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DlgDef::OnClose) );
}

DlgDef::~DlgDef()
{
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DlgDef::OnClose ) );
	
	CAL_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgDef::OnCalendarShow ), NULL, this );	
	PR_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgDef::PREvent ), NULL, this );
	NX_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgDef::NXEvent ), NULL, this );

}

CfgDlgDef::CfgDlgDef( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("General settings") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText17 = new wxStaticText( this, wxID_ANY, _("Speed"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	fgSizer3->Add( m_staticText17, 0, wxALL, 5 );
	
	
	sbSizer4->Add( fgSizer3, 1, wxEXPAND, 5 );
	
	
	bSizer2->Add( sbSizer4, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("TideFinder Interval") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );
	
	
	fgSizer4->Add( bSizer3, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	
	fgSizer4->Add( bSizer4, 1, wxEXPAND, 5 );
	
	
	sbSizer3->Add( fgSizer4, 1, wxEXPAND, 5 );
	
	
	bSizer2->Add( sbSizer3, 0, wxALL|wxEXPAND, 5 );
	
	
	bSizer1->Add( bSizer2, 1, wxEXPAND, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	
	bSizer1->Add( m_sdbSizer1, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );
	
	this->Centre( wxBOTH );
}

CfgDlgDef::~CfgDlgDef()
{
}

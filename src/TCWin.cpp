// For compilers that support precompilation, includes "wx.h".
//#ifndef  WX_PRECOMP
  #include "wx/wx.h"
//#endif //precompiled headers

#include "TCWin.h"
#include "wx/utils.h"
#include <wx/datetime.h>
#include <wx/string.h>
#include "TideFindergui_impl.h"
#include <wx/graphics.h>
#include "tcmgr.h"
#include <wx/wxchar.h>

class Harmonics;
class Dlg;

//using namespace std;


enum
{
      ID_TCWIN_NX,
      ID_TCWIN_PR
};

enum
{
            FORWARD_ONE_HOUR_STEP    =3600,
            FORWARD_TEN_MINUTES_STEP =600,
            FORWARD_ONE_MINUTES_STEP =60,
            BACKWARD_ONE_HOUR_STEP    =-3600,
            BACKWARD_TEN_MINUTES_STEP =-600,
            BACKWARD_ONE_MINUTES_STEP =-60
};

enum
{
      TIDE_PLOT
};

#include <wx/listimpl.cpp>
WX_DEFINE_LIST( SplineList );

BEGIN_EVENT_TABLE ( TCWin, wxWindow ) EVT_PAINT ( TCWin::OnPaint )
    EVT_SIZE ( TCWin::OnSize )
    EVT_BUTTON ( wxID_OK, TCWin::OKEvent )
    EVT_BUTTON ( ID_TCWIN_NX, TCWin::NXEvent )
    EVT_BUTTON ( ID_TCWIN_PR, TCWin::PREvent )
    EVT_CLOSE ( TCWin::OnCloseWindow )
    
END_EVENT_TABLE()

// Define a constructor
TCWin::TCWin( wxWindow * parent, int x, int y, int PortNo, wxString PortName, int graphday_00_s, wxDateTime graphDayD, wxString myUnits )
{
	m_passPort = PortNo;
	m_passName = PortName;
    m_passStation = graphday_00_s;
	m_passGraphDay = graphDayD;
	m_units = myUnits;

    //    As a display optimization....
    //    if current color scheme is other than DAY,
    //    Then create the dialog ..WITHOUT.. borders and title bar.
    //    This way, any window decorations set by external themes, etc
    //    will not detract from night-vision
	long wstyle = wxCLIP_CHILDREN | wxDEFAULT_DIALOG_STYLE|wxSIMPLE_BORDER ;

   
    wxDialog::Create( parent, wxID_ANY, wxString( _T ( "test" ) ), wxPoint( x, y ),
                      wxSize( 550, 480 ), wstyle );

	c_blue = wxColour(220, 220, 220);
	SetBackgroundColour(c_blue);
    pParent = parent;


    SetTitle( wxString( _( "Tide Finder" ) ) );
	 m_plot_type = TIDE_PLOT;


    int sx, sy;
    GetClientSize( &sx, &sy );
    int swx, swy;
    GetSize( &swx, &swy );
    int parent_sx, parent_sy;
    pParent->GetClientSize( &parent_sx, &parent_sy );

    int xc = x + 8;
    int yc = y;

//  Arrange for tcWindow to be always totally visible
    if( ( x + 8 + swx ) > parent_sx ) xc = xc - swx - 16;
    if( ( y + swy ) > parent_sy ) yc = yc - swy;

//  Don't let the window origin move out of client area
    if( yc < 0 ) yc = 0;
    if( xc < 0 ) xc = 0;

    wxPoint r( xc, yc );
    Move( r );

  	myDlg = new Dlg(pParent, wxID_ANY,  _T("Tide Finder"), wxDefaultPosition, wxSize( -1,-1 ), wxCLOSE_BOX|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
	
	myDlg->intPortNo = m_passPort;
	myDlg->m_graphday = m_graphday;
	myDlg->m_t_graphday_00_at_station = m_passStation;
	myDlg->m_graphday = m_passGraphDay;
	m_t_graphday_00_at_station = m_passStation;
	m_graphday = m_passGraphDay;

//    Figure out this computer timezone minute offset
    wxDateTime this_now = wxDateTime::Now();
    wxDateTime this_gmt = this_now.ToGMT();

#if wxCHECK_VERSION(2, 6, 2)
    wxTimeSpan diff = this_now.Subtract( this_gmt );
#else
    wxTimeSpan diff = this_gmt.Subtract ( this_now );
#endif

	int diff_mins = diff.GetMinutes();
	
	//  Correct a bug in wx3.0.2
	//  If the system TZ happens to be GMT, with DST active (e.g.summer in London),
	//  then wxDateTime returns incorrect results for toGMT() method
#if wxCHECK_VERSION(3, 0, 2)
	if (diff_mins == 0 && this_now.IsDST())
		diff_mins += 60;
#endif

    int station_offset = 0;

    m_corr_mins = station_offset - diff_mins;
    if( this_now.IsDST() ) m_corr_mins += 60;

//    Establish the inital drawing day as today
	
    btc_valid = false;

    wxString* TClist = NULL;
    m_tList = new wxListBox( this, -1, wxPoint( sx * 65 / 100, 11 ),
                             wxSize( ( sx * 30 / 100 ), ( sy * 20 / 100 ) ), 0, TClist,
                             wxLB_SINGLE | wxLB_NEEDED_SB );

    OK_button = new wxButton( this, wxID_OK, _( "OK" ), wxPoint( sx - 100, sy - 32 ),
                              wxDefaultSize );

    PR_button = new wxButton( this, ID_TCWIN_PR, _( "Prev" ), wxPoint( 10, sy - 32 ),
                              wxSize( 60, -1 ) );

    m_ptextctrl = new wxStaticText( this, -1, _T(""), wxPoint( sx * 5 / 100, 30 ),
                                  wxSize( ( sx * 55 / 100 ), ( sy *20 / 100 ) ) ,
                                  wxALIGN_CENTRE);
    int bsx, bsy, bpx, bpy;
    PR_button->GetSize( &bsx, &bsy );
    PR_button->GetPosition( &bpx, &bpy );

    NX_button = new wxButton( this, ID_TCWIN_NX, _( "Next" ), wxPoint( bpx + bsx + 5, bpy ),
                              wxSize( 60, -1 ) );

    //  establish some graphic element sizes/locations
    int x_graph = sx * 1 / 10;
    int y_graph = sy * 32 / 100;
    int x_graph_w = sx * 8 / 10;
    int y_graph_h = sy * 50 / 100;
    m_graph_rect = wxRect(x_graph, y_graph, x_graph_w, y_graph_h);


    // Build graphics tools

    pSFont = wxTheFontList->FindOrCreateFont( 8, wxFONTFAMILY_SWISS, wxNORMAL,
                                                    wxFONTWEIGHT_NORMAL, FALSE, wxString( _T ( "Arial" ) ) );
    pSMFont = wxTheFontList->FindOrCreateFont( 10, wxFONTFAMILY_SWISS, wxNORMAL,
                                                       wxFONTWEIGHT_NORMAL, FALSE, wxString( _T ( "Arial" ) ) );
    pMFont = wxTheFontList->FindOrCreateFont( 11, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD,
                                                      FALSE, wxString( _T ( "Arial" ) ) );
    pLFont = wxTheFontList->FindOrCreateFont( 12, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD,
                                                      FALSE, wxString( _T ( "Arial" ) ) );
	pVLFont = wxTheFontList->FindOrCreateFont( 12, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD,
                                                      FALSE, wxString( _T ( "Arial" ) ) );

    wxColour c_blue = wxColour(128, 248, 248);
	wxColour c_green = wxColour(0, 166, 80);
	wxColour c_yellow_orange = wxColour(253, 184, 19);
	wxColour c_orange = wxColour(248, 128, 64);
	
	c_black1 = wxColour(0, 0, 0);
	wxColour c_red = wxColour(248, 0, 0);
    c_grey = wxColour(236,233,216);
	c_grey2 = wxColour(212,208,200);

    pblack_1 = wxThePenList->FindOrCreatePen( c_black1, 1, wxSOLID );
                                                                         
    pblack_2 = wxThePenList->FindOrCreatePen( c_black1, 2,wxSOLID );
                                                                          
    pblack_3 = wxThePenList->FindOrCreatePen( c_black1, 1, wxSOLID );
                                                                         
    pred_2 = wxThePenList->FindOrCreatePen( c_red, 4, wxSOLID );
                                                                       
    pltgray = wxTheBrushList->FindOrCreateBrush( c_grey,wxSOLID );
                                                                               
    pltgray2 = wxTheBrushList->FindOrCreateBrush( c_grey2, wxSOLID );
                                                                               

    //  Fill in some static text control information
    //  Tidal station information
    // write the first line
   m_ptextctrl->SetFont( *pVLFont );
   m_ptextctrl->SetForegroundColour(c_black1);
   m_ptextctrl->SetBackgroundColour(c_blue);
   //m_ptextctrl->SetLabel(wxT("DOVER"));

}

TCWin::~TCWin()
{
    //Do Nothing;
}

void TCWin::OKEvent( wxCommandEvent& event )
{
    Hide();
    delete m_tList;
    Destroy();                          // that hurts
}

void TCWin::OnCloseWindow( wxCloseEvent& event )
{
    Hide();
    delete m_tList;

    Destroy();                          // that hurts
}

void TCWin::NXEvent( wxCommandEvent& event )
{
    wxTimeSpan dt( 24, 0, 0, 0 );
    m_graphday.Add( dt );
    wxDateTime dm = m_graphday;

    wxDateTime graphday_00 = dm.ResetTime();
    if(graphday_00.GetYear() == 2013)
        int yyp = 4;

    time_t t_graphday_00 = graphday_00.GetTicks();
    if( !graphday_00.IsDST() && m_graphday.IsDST() ) t_graphday_00 -= 3600;
    if( graphday_00.IsDST() && !m_graphday.IsDST() ) t_graphday_00 += 3600;
    m_t_graphday_00_at_station = t_graphday_00 - ( m_corr_mins * 60 );
	myDlg->m_t_graphday_00_at_station = m_t_graphday_00_at_station;
	myDlg->m_graphday = m_graphday;

    btc_valid = false;
    Refresh();
	
}

void TCWin::PREvent( wxCommandEvent& event )
{
    wxTimeSpan dt( -24, 0, 0, 0 );
    m_graphday.Add( dt );
    wxDateTime dm = m_graphday;

    wxDateTime graphday_00 = dm.ResetTime();
    time_t t_graphday_00 = graphday_00.GetTicks();

    if( !graphday_00.IsDST() && m_graphday.IsDST() ) t_graphday_00 -= 3600;
    if( graphday_00.IsDST() && !m_graphday.IsDST() ) t_graphday_00 += 3600;

    m_t_graphday_00_at_station = t_graphday_00 - ( m_corr_mins * 60 );
	myDlg->m_t_graphday_00_at_station = m_t_graphday_00_at_station;
	myDlg->m_graphday = m_graphday;

    btc_valid = false;
    Refresh();
}

void TCWin::Resize( void )
{
}

void TCWin::RePosition( void )
{
//    Position the window
    double lon = -5;
    double lat = 50;

    wxPoint r;
    Move( r );
}

void TCWin::OnPaint( wxPaintEvent& event )
{
	int x, y;
    int i;
    char sbuf[100];
    int w;
    float tcmax, tcmin;
    GetClientSize( &x, &y );

    wxPaintDC dc( this );

    {
        int x_textbox = x * 5 / 100;
        int y_textbox = 6;

        int x_textbox_w = x * 51 / 100;
        int y_textbox_h = y * 25 / 100;

        // box the location text & tide-current table
        dc.SetPen( *pblack_3 );
        dc.SetBrush( *pltgray2 );

		m_ptextctrl->SetFont(*pVLFont);
		m_ptextctrl->SetForegroundColour(c_black1);
		m_ptextctrl->SetBackgroundColour(c_blue);
		if (m_passName == wxT("")){m_passName = wxT("DOVER");}
		m_ptextctrl->SetLabel( m_passName );

        wxRect tab_rect = m_tList->GetRect();
        dc.DrawRoundedRectangle( tab_rect.x - 4, y_textbox, tab_rect.width + 8, y_textbox_h, 4 ); //tide-current table box

        //    Box the graph
        dc.SetPen( *pblack_1 );
        dc.SetBrush( *pltgray );
        dc.DrawRectangle( m_graph_rect.x, m_graph_rect.y, m_graph_rect.width, m_graph_rect.height );

        //    Horizontal axis
        dc.SetFont( *pSFont );
        for( i = 0; i < 25; i++ ) {
            int xd = m_graph_rect.x + ( ( i ) * m_graph_rect.width / 25 );
            dc.DrawLine( xd, m_graph_rect.y, xd, m_graph_rect.y + m_graph_rect.height + 5 );

            char sbuf[5];
            sprintf( sbuf, "%02d", i );
#ifdef __WXMSW__
            wxString sst;
            sst.Printf( _T("%02d"), i );
            dc.DrawRotatedText( sst, xd + ( m_graph_rect.width / 25 ) / 2, m_graph_rect.y + m_graph_rect.height + 8, 270. );
#else
            int x_shim = -12;
            dc.DrawText ( wxString ( sbuf, wxConvUTF8 ), xd + x_shim + ( m_graph_rect.width/25 ) /2, m_graph_rect.y + m_graph_rect.height + 8 );
#endif
        }

        //    Make a line for "right now"

		time_t t_now = m_graphday.GetTicks(); // now, in ticks

        float t_ratio = m_graph_rect.width * ( t_now - m_t_graphday_00_at_station ) / ( 25 * 3600 );

        //must eliminate line outside the graph (in that case put it outside the window)
        int xnow = ( t_ratio < 0 || t_ratio > m_graph_rect.width ) ? -1 : m_graph_rect.x + (int) t_ratio;
        dc.SetPen( *pred_2 );
        dc.DrawLine( xnow, m_graph_rect.y, xnow, m_graph_rect.y + m_graph_rect.height );
        dc.SetPen( *pblack_1 );
	
        //    Build the array of values, capturing max and min and HW/LW list
	    
        if( !btc_valid ) {

            float dir;
            tcmax = -10;
            tcmin = 10;
            float val;
            m_tList->Clear();
            int list_index = 0;
            bool wt;

            wxBeginBusyCursor();

            // get tide flow sens ( flood or ebb ? )
			myDlg->ptcmgr->GetTideFlowSens(m_t_graphday_00_at_station, BACKWARD_ONE_HOUR_STEP, m_passPort, tcv[0], val, wt);           			
			for( i = 0; i < 26; i++ ) {
                int tt = m_t_graphday_00_at_station + ( i * FORWARD_ONE_HOUR_STEP );
				myDlg->ptcmgr->GetTideOrCurrent( tt, m_passPort, tcv[i], dir );

                if( tcv[i] > tcmax ) tcmax = tcv[i];

                if( tcv[i] < tcmin ) tcmin = tcv[i];
                
				if( TIDE_PLOT == m_plot_type ) {
                    if( !( ( tcv[i] > val ) == wt ) )                // if tide flow sens change
                    {
                        float tcvalue;                                  //look backward for HW or LW
                        time_t tctime;
                        myDlg->ptcmgr->GetHightOrLowTide(tt, BACKWARD_TEN_MINUTES_STEP, BACKWARD_ONE_MINUTES_STEP, tcv[i], wt, m_passPort, tcvalue, tctime);                       

                        wxDateTime tcd;                                                 //write date
                        wxString s, s1;
                        tcd.Set( tctime + ( m_corr_mins * 60 ) );
                        s.Printf( tcd.Format( _T("%H:%M  ") ) );
                        s1.Printf( _T("%05.2f "), tcvalue );                           //write value
                        s.Append( s1 );
                        s.Append( m_units);

                        ( wt ) ? s.Append( _("  HW") ) : s.Append( _("  LW") );         //write HW or LT

                        m_tList->Insert( s, list_index );                       // update table list
                        list_index++;

                        wt = !wt;                                            //change tide flow sens
                    }
                    val = tcv[i];
                }
            }

            wxEndBusyCursor();

//    Set up the vertical parameters based on Tide or Current plot
  
                ib = (int) tcmin;
                if( tcmin < 0 ) ib -= 1;
                it = (int) tcmax + 1;

                im = it - ib; //abs ( ib ) + abs ( it );
                m_plot_y_offset = ( m_graph_rect.height * ( it - ib ) ) / im;
                val_off = ib;


//    Build spline list of points

            m_sList.DeleteContents( true );
            m_sList.Clear();

            for( i = 0; i < 26; i++ ) {
                wxPoint *pp = new wxPoint;
                pp->x = m_graph_rect.x + ( ( i ) * m_graph_rect.width / 25 );
                pp->y = m_graph_rect.y + ( m_plot_y_offset )
                - (int) ( ( tcv[i] - val_off ) * m_graph_rect.height / im );

                m_sList.Append( pp );
            }

            btc_valid = true;
        }
		wxColour c_black1 = wxColour(0, 0, 0);
        dc.SetTextForeground(c_black1);

        //    Vertical Axis

        //      Maybe skip some lines and legends if the range is too high
        int height_stext;
         dc.GetTextExtent( _T("1"), NULL, &height_stext );

        int i_skip = 1;
        if( height_stext > m_graph_rect.height / im ) i_skip = 2;

        i = ib;
        while( i < it + 1 ) {
            int yd = m_graph_rect.y + ( m_plot_y_offset ) - ( ( i - val_off ) * m_graph_rect.height / im );

            if( ( m_plot_y_offset + m_graph_rect.y ) == yd ) dc.SetPen( *pblack_2 );
            else
                dc.SetPen( *pblack_1 );

            dc.DrawLine( m_graph_rect.x, yd, m_graph_rect.x + m_graph_rect.width, yd );
#ifdef __WXMSW__
            _snprintf( sbuf, 99, "%d", i );
#else
            snprintf( sbuf, 99, "%d", i );
#endif
            dc.DrawText( wxString( sbuf, wxConvUTF8 ), m_graph_rect.x - 20, yd - 5 );
            i += i_skip;

        }

        //    Draw the Value curve
#if wxCHECK_VERSION(2, 9, 0)
        wxPointList *list = (wxPointList *)&m_sList;
#else
        wxList *list = (wxList *) &m_sList;
#endif

        dc.SetPen(  *pblack_2);
#if wxUSE_SPLINES
        dc.DrawSpline( list );
#else
        dc.DrawLines ( list );
#endif
        //  More Info

///
        int station_offset = 0;
        int h = station_offset / 60;
        int m = station_offset - ( h * 60 );
        if( m_graphday.IsDST() ) h += 1;
        m_stz.Printf( _T("Z %+03d:%02d"), h, m );

        dc.SetFont( *pSFont );
        dc.GetTextExtent( m_stz, &w, &h );
        dc.DrawText( m_stz, x / 2 - w / 2, y * 88 / 100 );

        // There seems to be some confusion about format specifiers
        //  Hack this.....
        //  Find and use the longest "sprintf" result......
        wxString sdate;
        wxString s1 = m_graphday.Format( _T ( "%#x" ) );
        wxString s2 = m_graphday.Format( _T ( "%x" ) );

        if( s2.Len() > s1.Len() ) sdate = s2;
        else
            sdate = s1;
        dc.SetFont( *pMFont );
        dc.GetTextExtent( sdate, &w, &h );
        dc.DrawText( sdate, x / 2 - w / 2, y * 92 / 100 );

        ///
       
            dc.GetTextExtent(  wxT("meters"), &w, &h );
            dc.DrawRotatedText(  wxT("meters"), 5,
                                m_graph_rect.y + m_graph_rect.height / 2 + w / 2, 90. );
  

//    Today or tomorrow
        wxString sday;
        wxDateTime this_now = wxDateTime::Now();

        int day = m_graphday.GetDayOfYear();
        if( m_graphday.GetYear() == this_now.GetYear() ) {
            if( day == this_now.GetDayOfYear() ) sday.Append( _( "Today" ) );
            else if( day == this_now.GetDayOfYear() + 1 ) sday.Append( _( "Tomorrow" ) );
            else
                sday.Append( m_graphday.GetWeekDayName( m_graphday.GetWeekDay() ) );
        } else if( m_graphday.GetYear() == this_now.GetYear() + 1
                   && day == this_now.Add( wxTimeSpan::Day() ).GetDayOfYear() ) sday.Append(
                           _( "Tomorrow" ) );

        dc.SetFont( *pSFont );
        dc.GetTextExtent( sday, &w, &h );
        dc.DrawText( sday, 55 - w / 2, y * 88 / 100 );

    }
}


void TCWin::OnSize( wxSizeEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    int x, y;
    GetPosition( &x, &y );
}


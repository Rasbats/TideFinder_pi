/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  TideFinder Plugin
 * Author:   Mike Rossiter
 *
 ***************************************************************************
 *   Copyright (C) 2013 by Mike Rossiter                                   *
 *   $EMAIL$                                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#include "TideFindergui_impl.h"
#include <wx/progdlg.h>
#include <wx/wx.h>
#include "wx/dir.h"
#include <list>
#include <cmath>
#include "TideFinder_pi.h"
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>
#include "wx/dialog.h"
#include <wx/datetime.h>
#include "TCWin.h"
#include "wx/string.h"
#include <list>
#include <vector>
#include "timectrl.h"
#include "tcmgr.h"

class Position;
class TideFinder_pi;
class Harmonics;
class TidalFactors;
class PortTides;
class TCWin;
class TCMgr;

using namespace std;
// convert degrees to radians  
static inline double DegToRad(double deg) { return (deg * M_PI) / 180.0; }
// event handlers


/* WARNING:  These values are very important, as used under the "default" case. */
#define INT_PART 3
#define DEC_PART 2

static wxString port_clicked;

enum
{
            FORWARD_ONE_HOUR_STEP    =3600,
            FORWARD_TEN_MINUTES_STEP =600,
            FORWARD_ONE_MINUTES_STEP =60,
            BACKWARD_ONE_HOUR_STEP    =-3600,
            BACKWARD_TEN_MINUTES_STEP =-600,
            BACKWARD_ONE_MINUTES_STEP =-60
};

double Str2LatLong(char* coord){

    int sign = +1;
    double val;

    int i = 0;  /* an index into coord, the text-input string, indicating the character currently being parsed */

    int p[9] = {0,0,1,  /* degrees */
                0,0,1,  /* minutes */
                0,0,1   /* seconds */
               };
    int* ptr = p;   /* p starts at Degrees. 
                       It will advance to the Decimal part when a decimal-point is encountered,
                       and advance to minutes & seconds when a separator is encountered */
    int  flag = INT_PART; /* Flips back and forth from INT_PART and DEC_PART */

    while(1)
    {
        switch (coord[i])
        {
            /* Any digit contributes to either degrees,minutes, or seconds */
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                *ptr = 10* (*ptr) + (coord[i] - '0');
                if (flag == DEC_PART)  /* it'd be nice if I could find a clever way to avoid this test */
                {
                    ptr[1] *= 10;
                }
                break;

            case '.':     /* A decimal point implies ptr is on an integer-part; advance to decimal part */
                flag = DEC_PART; /* after encountering a decimal point, we are now processing the Decimal Part */
                ptr++;  /* ptr[0] is now the Decimal piece; ptr[1] is the Denominator piece (powers of 10) */
                break;

            /* A Null terminator triggers return (no break necessary) */
            case '\0':
                val = p[0]*3600 + p[3]*60 + p[6];             /* All Integer math */
                if (p[1]) val += ((double)p[1]/p[2]) * 3600;  /* Floating-point operations only if needed */
                if (p[4]) val += ((double)p[4]/p[5]) *   60;  /* (ditto) */
                if (p[7]) val += ((double)p[7]/p[8]);         /* (ditto) */
                return sign * val / 3600.0;                 /* Only one floating-point division! */

            case 'W':
            case 'S':
                sign = -1;
                break;

            /* Any other symbol is a separator, and moves ptr from degrees to minutes, or minutes to seconds */
            default:
                /* Note, by setting DEC_PART=2 and INT_PART=3, I avoid an if-test. (testing and branching is slow) */
                ptr += flag;
                flag = INT_PART; /* reset to Integer part, we're now starting a new "piece" (degrees, min, or sec). */
        }
        i++;
    }

    return -1.0;  /* Should never reach here! */
}



CfgDlg::CfgDlg( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : CfgDlgDef( parent, id, title, pos, size, style )
{
	
}

Dlg::Dlg( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DlgDef( parent, id, title, pos, size, style )
{	
    this->Fit();
	LoadHarmonics();
	btc_valid = true;
	outOfRadius = true;
    dbg=false; //for debug output set to true
}

void Dlg::OnClose(wxCloseEvent& event)
{
	wxString to_delete = _T("T") + m_PortNo;
	DeleteSingleWaypoint( to_delete );
	plugin->OnTideFinderDialogClose();
}

void Dlg::OnContextMenu(double m_lat, double m_lon){

	

	GetPortDialog aboutDialog ( this, -1, _("Select your Port"),
	                          wxPoint(200, 200), wxSize(300, 200), wxRESIZE_BORDER);

    
	aboutDialog.dialogText->InsertColumn(0, _T(""), 0 , wxLIST_AUTOSIZE);
	aboutDialog.dialogText->SetColumnWidth(0, 290);
	aboutDialog.dialogText->InsertColumn(1, _T(""), 0 , wxLIST_AUTOSIZE);
	aboutDialog.dialogText->SetColumnWidth(1, 0);
	aboutDialog.dialogText->DeleteAllItems();

	m_staticText2->SetLabel(wxT(""));

    bool foundPort = false;
	outOfRadius = false;
	double radius = 0.1;
	double dist = 0;
	char N = 'N';
	int c = 0;
	wxString dimensions = wxT(""), s;
	wxString exPort = wxT("");
		
	wxListItem     row_info;  
	wxString       cell_contents_string = wxEmptyString;

	double lat = 50;
	double lon = -4;

	bool newItem = false;

	int i;
	
	while (!foundPort){
	        for ( i=1 ; i<ptcmgr->Get_max_IDX() +1 ; i++ )
            {				
						pIDX = ptcmgr->GetIDX_entry (i);

                        char type = pIDX->IDX_type;             // Entry "TCtcIUu" identifier
                        if ( ( type == 't' ) ||  ( type == 'T' ) )  // only Tides
                        {                              

								lat = pIDX->IDX_lat;
								lon = pIDX->IDX_lon;

				 				dist = distance(lat,lon,m_lat,m_lon, N);
				     			if (dist < radius){
									wxString locn( pIDX->IDX_station_name, wxConvUTF8 );
									wxString locna, locnb;
									if( locn.Contains( wxString( _T ( "," ) ) ) ) {
										locna = locn.BeforeFirst( ',' );
										locnb = locn.AfterFirst( ',' );
									} else {
										locna = locn;
										locnb.Empty();
									}
									m_PortName = locna;
								    intPortNo = pIDX->IDX_rec_num ; 
								    m_PortNo = wxString::Format(wxT("%i"),intPortNo);
								    newItem = true;								  
							    }							  
						}											
							
				bool inList = false;

				int g = aboutDialog.dialogText->GetItemCount();
			    int p = 0; 
				for (p;p<g; p++){
 
							   // Set what row it is (m_itemId is a member of the regular wxListCtrl class)
							   row_info.m_itemId = p;
							   // Set what column of that row we want to query for information.
							   row_info.m_col = 1;
							   // Set text mask
							   row_info.m_mask = wxLIST_MASK_TEXT;
							   // Get the info and store it in row_info variable.   
							   aboutDialog.dialogText->GetItem( row_info );
							   // Extract the text out that cell
							   cell_contents_string = row_info.m_text; 
							
							   if (cell_contents_string == m_PortNo){								 
								  inList = true;								  
							   }						
				    }

						 if (!inList && newItem){
							wxListItem myItem;
							
							aboutDialog.dialogText->InsertItem(c,myItem);
							aboutDialog.dialogText->SetItem(c,0,m_PortName);
							aboutDialog.dialogText->SetItem(c,1,m_PortNo);
							wxString myGUID = _T("T") + m_PortNo;
							myWP = new PlugIn_Waypoint(lat,lon,_T("circle"),m_PortName,myGUID);
							AddSingleWaypoint(myWP,false);
							
							newItem = false;

							c++;
							if (c == 4){
									foundPort = true;
									outOfRadius = false;
									break;
							}	
						}
					  
					     
			}
        
		radius = radius + 5;
		if (radius > 200) {
			wxString notFound = _("No Tidal Stations found within 200NM");
			wxListItem myEmptyItem;
			aboutDialog.dialogText->InsertItem(0, myEmptyItem);
			aboutDialog.dialogText->SetItem(0, 0, notFound);
			outOfRadius = true;
			break;
		}
		i = 1;
	 }
	 
	this->m_parent->Refresh();

	long si = -1;
	long itemIndex = -1;     
	int f = 0;

	 if ( aboutDialog.ShowModal() != wxID_OK ){
		 for (f;f<4;f++) {
         itemIndex = aboutDialog.dialogText->GetNextItem(itemIndex,
                                         wxLIST_NEXT_ALL,
                                         wxLIST_STATE_SELECTED);
		 si = f;
		 row_info.m_itemId = si;
			   // Set what column of that row we want to query for information.
			   row_info.m_col = 1;
			   // Set text mask
			   row_info.m_mask = wxLIST_MASK_TEXT;
			   // Get the info and store it in row_info variable.   
			   aboutDialog.dialogText->GetItem( row_info );
			   // Extract the text out that cell
			   m_PortNo = row_info.m_text;			  
			    // Delete the waypoint
			   wxString t = _T("T") + m_PortNo;   
			   DeleteSingleWaypoint(t);

			   CAL_button->Hide();
			   PR_button->Hide();
			   NX_button->Hide();
			   GF_button->Hide();
			   m_listBox1->Hide();
			   m_staticText3->Hide();
			   m_staticText2->SetLabel(wxT("Right-Click in the location for tide prediction\n ... and click \'Tide Finder Position\' from the menu.\n\n DO NOT close this box!!!"));
			   this->Refresh();
			   this->Fit();
		 }
	 }
	 else{
		 m_listBox1->Show();
		 m_staticText3->Show();
		 CAL_button->Show();
		 PR_button->Show();
	     NX_button->Show();
	     GF_button->Show();
		 
		 foundPort = false;
		 
		 for (f;f<4;f++) {
			   itemIndex = aboutDialog.dialogText->GetNextItem(itemIndex,
											 wxLIST_NEXT_ALL,
											 wxLIST_STATE_SELECTED);
			   si = f;

			   row_info.m_itemId = si;
			   // Set what column of that row we want to query for information.
			   row_info.m_col = 1;
			   // Set text mask
			   row_info.m_mask = wxLIST_MASK_TEXT;
 
			   // Get the info and store it in row_info variable.   
			   aboutDialog.dialogText->GetItem( row_info );
			   // Extract the text out that cell
			   m_PortNo = row_info.m_text;
			    // Delete the waypoint
			   wxString t = _T("T") + m_PortNo;
			   SelectedPorts[f] = t;			   
		 }

         for (;;) {
         itemIndex = aboutDialog.dialogText->GetNextItem(itemIndex,
                                         wxLIST_NEXT_ALL,
                                         wxLIST_STATE_SELECTED);
 
         if (itemIndex == -1) break;
		
		 // Got the selected item index
		  if (aboutDialog.dialogText->IsSelected(itemIndex)){
			  si = itemIndex;
			  foundPort = true;
			  break;			  					 		
		   } 
		 }
		   if (foundPort){
			   // Set what row it is (m_itemId is a member of the regular wxListCtrl class)
			   row_info.m_itemId = si;
			   // Set what column of that row we want to query for information.
			   row_info.m_col = 1;
			   // Set text mask
			   row_info.m_mask = wxLIST_MASK_TEXT;
 
			   // Get the info and store it in row_info variable.   
			   aboutDialog.dialogText->GetItem( row_info );		  
			   // Extract the text out that cell
			   m_PortNo = row_info.m_text;
			   // Delete the other waypoints
			   f = 0;
			   for (f;f<4;f++) {
					wxString sp = SelectedPorts[f];
			      if (_T("T") + m_PortNo != sp) 
			         DeleteSingleWaypoint(sp);
			   }
               intPortNo = wxAtoi(m_PortNo);
			   row_info.m_col = 0;

			   aboutDialog.dialogText->GetItem( row_info );
			   m_PortName = row_info.m_text;
			   m_staticText2->SetLabel(m_PortName);
			   btc_valid = true;  // Start with today's tides
               OnCalculate();
			   this->Refresh();
			   this->Fit();
		  }
		  else{		
			   if (!outOfRadius) {
				   wxMessageBox(wxT("No port selected\n ...Right-click to select again"), _T("Port not selected"));
			   }
				   m_listBox1->Hide();
				   m_staticText3->Hide();
				   m_staticText2->SetLabel(_T("Right-click to select again ..."));
				   this->Refresh();
				   this->Fit();
			   
		   }

	}
}


#define pi 3.14159265358979323846
double Dlg::distance(double lat1, double lon1, double lat2, double lon2, char unit) {
 double theta, dist;
 theta = lon1 - lon2;
 dist = sin(deg2rad(lat1)) * sin(deg2rad(lat2)) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta));
 dist = acos(dist);
 dist = rad2deg(dist);
 dist = dist * 60 * 1.1515;
 switch(unit) {
case 'M':
 break;
case 'K':
 dist = dist * 1.609344;
 break;
case 'N':
 dist = dist * 0.8684;
 break;
 }
 return (dist);
}
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:: This function converts decimal degrees to radians :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double Dlg::deg2rad(double deg) {
 return (deg * pi / 180);
}
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:: This function converts radians to decimal degrees :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double Dlg::rad2deg(double rad) {
 return (rad * 180 / pi);
}

void Dlg::LoadHarmonics()
{
	  //  Establish a "home" location
        
	  g_SData_Locn = *GetpSharedDataLocation();

      // Establish location of Tide and Current data
      pTC_Dir = new wxString(_T("tcdata"));
      pTC_Dir->Prepend(g_SData_Locn);
      pTC_Dir->Append(wxFileName::GetPathSeparator());  
	
      wxString TCDir;
      TCDir = *pTC_Dir;
      
      wxLogMessage(_T("Using Tide/Current data from:  ") + TCDir);
	  wxString cache_locn = TCDir; 

	  wxString harm2test = TCDir;
      harm2test.Append( _T("HARMONIC") );
	
	  ptcmgr = new TCMgr(TCDir, cache_locn);     	
}

void Dlg::OnCalculate(){

	pIDX = ptcmgr->GetIDX_entry(intPortNo);

	station_lat = pIDX->IDX_lat;
	
	//    Figure out this computer timezone minute offset
	wxDateTime this_now = wxDateTime::Now();
	wxDateTime this_gmt = this_now.ToGMT();

#if wxCHECK_VERSION(2, 6, 2)
	wxTimeSpan diff = this_now.Subtract(this_gmt);
#else
	wxTimeSpan diff = this_gmt.Subtract(this_now);
#endif

	int diff_mins = diff.GetMinutes();

	//  Correct a bug in wx3.0.2
	//  If the system TZ happens to be GMT, with DST active (e.g.summer in London),
	//  then wxDateTime returns incorrect results for toGMT() method
#if wxCHECK_VERSION(3, 0, 2)
	if (diff_mins == 0 && this_now.IsDST())
		diff_mins += 60;
#endif

	station_offset = ptcmgr->GetStationTimeOffset(pIDX);
	//wxMessageBox(wxString::Format(_T("%i"), station_offset));
	
	m_corr_mins = station_offset - diff_mins;
	if (this_now.IsDST()) m_corr_mins += 60;

	//    Establish the inital drawing day as today
	m_graphday = wxDateTime::Now();
	wxDateTime graphday_00 = wxDateTime::Today();

	graphday_00.ResetTime();

	time_t t_graphday_00 = graphday_00.GetTicks();

	//    Correct a Bug in wxWidgets time support
	if (!graphday_00.IsDST() && m_graphday.IsDST()) t_graphday_00 -= 3600;
	if (graphday_00.IsDST() && !m_graphday.IsDST()) t_graphday_00 += 3600;

	m_t_graphday_00_at_station = t_graphday_00 - (m_corr_mins * 60);

	//MakeLabelDate(station_offset, station_lat, m_graphday);

	CalcHWLW(intPortNo);  //port_clicked); 	
}


void Dlg::CalcHWLW(int PortCode)
{
	    
	
		m_listBox1->Clear();

		m_PortNo = wxString::Format(wxT("%i"),PortCode);

		float dir;
		
		int i, c, n, e;
		c = 0;
		e = 0;
		double myArrayOfRanges[8];

		float tcmax, tcmin;
		//float dir;
                        
		tcmax = -10;
        tcmin = 10;
        
		float val = 0;
		int list_index = 0 ;
		int array_index = 0;
        bool  wt = 0;
		float myLW, myHW;

		Station_Data *pmsd;
		wxString sHWLW = _T("");
		

                        // get tide flow sens ( flood or ebb ? )
						
                        ptcmgr->GetTideFlowSens(m_t_graphday_00_at_station, BACKWARD_ONE_HOUR_STEP, pIDX->IDX_rec_num, tcv[0], val, wt);
		
						for ( i=0 ; i<26 ; i++ )
                        {
                                int tt = m_t_graphday_00_at_station + ( i * FORWARD_ONE_HOUR_STEP );
                                ptcmgr->GetTideOrCurrent ( tt, pIDX->IDX_rec_num, tcv[i], dir );
								
                                if ( tcv[i] > tcmax )
                                        tcmax = tcv[i];

                                                if ( tcv[i] < tcmin )
                                                   tcmin = tcv[i];                                                
                                                    if ( ! ((tcv[i] > val) == wt) )                // if tide flow sens change
                                                    {
                                                      float tcvalue;                                        //look backward for HW or LW
                                                      time_t tctime;
                                                      ptcmgr->GetHightOrLowTide(tt, BACKWARD_TEN_MINUTES_STEP, BACKWARD_ONE_MINUTES_STEP, tcv[i], wt, pIDX->IDX_rec_num, tcvalue, tctime);
													  
													  wxDateTime tcd;                                                 //write date
													  wxString s, s1;
													  tcd.Set(tctime + (m_corr_mins * 60));
													  s.Printf(tcd.Format(_T("%H:%M  ")));
													  s1.Printf(_T("%05.2f "), tcvalue);                           //write value
													  s.Append(s1);
													  Station_Data *pmsd = pIDX->pref_sta_data;                       //write unit
													  if (pmsd) s.Append(wxString(pmsd->units_abbrv, wxConvUTF8));
													  myUnits = wxString(pmsd->units_conv, wxConvUTF8);
													  s.Append(_T("   "));
													  (wt) ? s.Append(_("HW")) : s.Append(_("LW"));         //write HW or LT

													  m_listBox1->Insert(s, list_index);                       // update table list
													  list_index++;
													  wt = !wt;
                                                    }

													val = tcv[i];                                                                                                
                        }

										
}


int Dlg::FindPortID(wxString myPort)
{	int t;
	        for ( int i=1 ; i<ptcmgr->Get_max_IDX() +1 ; i++ )
            {				
						IDX_entry *pIDX = ptcmgr->GetIDX_entry (i);

                        char type = pIDX->IDX_type;             // Entry "TCtcIUu" identifier
                        if ( ( type == 't' ) ||  ( type == 'T' ) )  // only Tides
                        {                              
							  wxString s = wxString(pIDX->IDX_reference_name,wxConvUTF8); 
							  if ( s == myPort)
							  {		
								  return i;
							  }							  
						}
						t = i;
			}
			return 0;
}
/*
void Dlg::MakeLabelDate(int offset, double lat, wxDateTime graphday) {


	int h = offset / 60;
	int m = offset - (h * 60);
	if (graphday.IsDST()) h += 1;
	m_stz.Printf(_T("UTC %+03d:%02d"), h, m);

	//    Make the "nice" (for the US) station time-zone string, brutally by hand	
	if (lat > 20.0) {
		wxString mtz;
		switch (offset) {
		case -240:
			mtz = _T("AST");
			break;
		case -300:
			mtz = _T("EST");
			break;
		case -360:
			mtz = _T("CST");
			break;
		}

		if (mtz.Len()) {
			if (graphday.IsDST()) mtz[1] = 'D';

			m_stz = mtz;
		}
	}

	wxString sdate;
	sdate = graphday.Format(_T("%A %d %b %Y"));
	wxString labeldate;
	labeldate = sdate + _T(" ") + m_stz + _T("");
	m_staticText3->SetLabel(labeldate);



}
*/
void Dlg::NXEvent( wxCommandEvent& event )
{	
	if (outOfRadius) return;
	wxTimeSpan dt( 24, 0, 0, 0 );
    m_graphday.Add( dt );

	//MakeLabelDate(station_offset, station_lat, m_graphday);

    wxDateTime dm = m_graphday;
    wxDateTime graphday_00 = dm.ResetTime();

    if(graphday_00.GetYear() == 2013)
        int yyp = 4;

    time_t t_graphday_00 = graphday_00.GetTicks();
    if( !graphday_00.IsDST() && m_graphday.IsDST() ) t_graphday_00 -= 3600;
    if( graphday_00.IsDST() && !m_graphday.IsDST() ) t_graphday_00 += 3600;
    m_t_graphday_00_at_station = t_graphday_00 - ( m_corr_mins * 60 );

    btc_valid = false;
	CalcHWLW(intPortNo);
}

void Dlg::PREvent( wxCommandEvent& event )
{
	if (outOfRadius) return;
	wxTimeSpan dt( -24, 0, 0, 0 );
    m_graphday.Add( dt );

	//MakeLabelDate(station_offset, station_lat, m_graphday);

    wxDateTime dm = m_graphday;
    wxDateTime graphday_00 = dm.ResetTime();
    time_t t_graphday_00 = graphday_00.GetTicks();

    if( !graphday_00.IsDST() && m_graphday.IsDST() ) t_graphday_00 -= 3600;
    if( graphday_00.IsDST() && !m_graphday.IsDST() ) t_graphday_00 += 3600;

    m_t_graphday_00_at_station = t_graphday_00 - ( m_corr_mins * 60 );

    btc_valid = false;
    CalcHWLW(intPortNo);
}

void Dlg::OnCalendarShow( wxCommandEvent& event )
{	
	if (outOfRadius) return;
	CalendarDialog CalDialog ( this, -1, _("Select the date for Tides"),
	                          wxPoint(100, 100), wxSize(240, 250) );	

	if ( CalDialog.ShowModal() == wxID_OK ){				

		m_graphday = CalDialog.dialogCalendar->GetDate();

		//MakeLabelDate(station_offset, station_lat, m_graphday);

		wxDateTime dm = m_graphday;

		wxDateTime graphday_00 = dm.ResetTime();

		time_t t_graphday_00 = graphday_00.GetTicks();
		if( !graphday_00.IsDST() && m_graphday.IsDST() ) t_graphday_00 -= 3600;
		if( graphday_00.IsDST() && !m_graphday.IsDST() ) t_graphday_00 += 3600;
		m_t_graphday_00_at_station = t_graphday_00 - ( m_corr_mins * 60 );
		
		btc_valid = false;
		CalcHWLW(intPortNo);
	}	
}

void Dlg::GFEvent(wxCommandEvent& event){

	if (outOfRadius) return;
	TCWin *myTCWin = new TCWin(this,100,100, intPortNo, m_PortName, m_t_graphday_00_at_station, m_graphday, station_offset, station_lat, myUnits);
	myTCWin->Show();

}

void Dlg::CalcMyTimeOfDay(){

	wxDateTime this_now = wxDateTime::Now();

	wxString todayHours = this_now.Format(_T("%H"));
	wxString todayMinutes = this_now.Format(_T("%M"));
	
	double h;
	double m;

	todayHours.ToDouble(&h);
	todayMinutes.ToDouble(&m);
	wxTimeSpan myTimeOfDay = wxTimeSpan(h,m,0,0);	
}

CalendarDialog::CalendarDialog ( wxWindow * parent, wxWindowID id, const wxString & title,
                           const wxPoint & position, const wxSize & size, long style )
: wxDialog( parent, id, title, position, size, style)
{
		
	wxString dimensions = wxT(""), s;
	wxPoint p;
	wxSize  sz;
 
	sz.SetWidth(220);
	sz.SetHeight(150);
	
	p.x = 6; p.y = 2;
	s.Printf(_(" x = %d y = %d\n"), p.x, p.y);
	dimensions.append(s);
	s.Printf(_(" width = %d height = %d\n"), sz.GetWidth(), sz.GetHeight());
	dimensions.append(s);
	dimensions.append(wxT("here"));
 
    dialogCalendar = new wxCalendarCtrl(this, -1, wxDefaultDateTime, p, sz, wxCAL_SHOW_HOLIDAYS ,wxT("Tide Calendar"));
	
	//wxWindowID text;

	//m_staticText = new wxStaticText(this,text,wxT("Time:"),wxPoint(15,155),wxSize(60,21));

	//wxDateTime dt = dt.Now();
	//_timeCtrl = new wxTimePickerCtrl(this, wxID_ANY, dt, wxPoint(75, 155), wxSize(80, 21));
		
	p.y += sz.GetHeight() + 30;
	wxButton * b = new wxButton( this, wxID_OK, _("OK"), p, wxDefaultSize );
	p.x += 110;
	wxButton * c = new wxButton( this, wxID_CANCEL, _("Cancel"), p, wxDefaultSize );
    
}


GetPortDialog::GetPortDialog ( wxWindow * parent, wxWindowID id, const wxString & title,
                           const wxPoint & position, const wxSize & size, long style )
: wxDialog( parent, id, title, position, size, style)
{
	
	wxString dimensions = wxT(""), s;
	wxPoint p;
	wxSize  sz;
 
	sz.SetWidth(size.GetWidth() - 20);
	sz.SetHeight(size.GetHeight() - 70);
 
	p.x = 6; p.y = 2;
 
	dialogText = new wxListView(this, wxID_ANY, p, sz, wxLC_NO_HEADER|wxLC_REPORT|wxLC_SINGLE_SEL, wxDefaultValidator, wxT(""));

    wxFont *pVLFont = wxTheFontList->FindOrCreateFont( 12, wxFONTFAMILY_SWISS, wxNORMAL, wxFONTWEIGHT_NORMAL ,
                                                      FALSE, wxString( _T ( "Arial" ) ) );
	dialogText->SetFont(*pVLFont);

	p.y += sz.GetHeight() + 10;

	p.x += 30;
	wxButton * b = new wxButton( this, wxID_OK, _("OK"), p, wxDefaultSize );
	p.x += 140;
	wxButton * c = new wxButton( this, wxID_CANCEL, _("Cancel"), p, wxDefaultSize );
};


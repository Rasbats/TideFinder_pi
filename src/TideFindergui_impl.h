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

#ifndef _CALCULATORGUI_IMPL_H_
#define _CALCULATORGUI_IMPL_H_


#ifdef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "TideFindergui.h"
#include "TideFinder_pi.h"
#include "NavFunc.h"
#include "tinyxml.h"
//#include "bbox.h"
#include "wx/math.h"
#include "wx/dialog.h"
#include <wx/calctrl.h>
#include "wx/window.h"
#include "timectrl.h"
#include "TCWin.h"
#include <wx/listctrl.h>
#include <list>
#include <vector>
//#include "resource.h"
//#include "TC_Error_Code.h"
#include "tcmgr.h"
#include <wx/choice.h>
#include <vector>


//using namespace std;

class TideFinder_pi;
class StandardPort;
class Position;
class Route;
class VMHData;
class TCWin;
class TCMgr;
class PortTides;
class RoutePoint

{
public:
      RoutePoint(double lat, double lon, const wxString& icon_ident, const wxString& name, bool bAddToList = true);
      RoutePoint( RoutePoint* orig );
      RoutePoint();
      ~RoutePoint(void);      
      void ReLoadIcon(void);

      wxDateTime GetCreateTime(void);
      void SetCreateTime( wxDateTime dt );
      
      void SetPosition(double lat, double lon);
      double GetLatitude()  { return m_lat; };
      double GetLongitude() { return m_lon; };
	  double m_lat;
	  double m_lon;


};

class CfgDlg : public CfgDlgDef
{
public:
      CfgDlg( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("TideFinder preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE );
};

class Dlg : public DlgDef
{
public:
        Dlg( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Tide Finder"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCLOSE_BOX|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );


        TideFinder_pi *plugin; 
		
		std::vector<VMHData> my_dataList;
		std::vector<VMHData> my_LocationList;


		void OnContextMenu(double m_lat, double m_lon);

		void SaveHarbourXMLtoDataClass();
		double distance(double lat1, double lon1, double lat2, double lon2, char unit);
		double deg2rad(double deg);
		double rad2deg(double rad);

		double m_lat;
		double m_lon;
		

		void OnCalculate();  //(wxCommandEvent& event);

	void PREvent( wxCommandEvent& event );
	void NXEvent( wxCommandEvent& event );
	void OnCalendarShow( wxCommandEvent& event );
	void GFEvent(wxCommandEvent& event);

	wxTimeSpan  myTimeOfDay;
	void CalcMyTimeOfDay();

	TCWin *myTCWin;

	int         m_t_graphday_00_at_station;
    wxDateTime  m_graphday;
	time_t myTime;
	
	bool LoadStandardPorts();
	PortTides PopulatePortTides(wxString PortName);

	void LoadHarmonics();
	int FindPortIDUsingChoice(wxString inPortName);

	wxArrayString    TideCurrentDataSet;
	std::vector<Position>my_positions;
	std::vector<StandardPort>my_ports;
	wxString wxPortName[100][5];
	wxString selectedPort;
	int intPortNo;
    wxString m_PortNo;
	
    TCMgr           *ptcmgr;
	IDX_entry *pIDX;
	wxString myUnits;

private:
		PlugIn_Waypoint *myWP;
		wxString SelectedPorts[3];

		int FindPortID(wxString myPort);		
		int FindNearestPorts(int n);

		void OnStartSetupHW();
		void CalcHWLW(int PortCode);
		void SetCorrectHWSelection();

		wxString    euTC[8][3];  //Date.Time, Height, Units, HW.LW
		wxString nearestHW[8];
		double myRange;

		
	    wxString         g_SData_Locn;
		
		wxString        *pTC_Dir;

	    wxWindow          *m_parent_window;
		void OnClose( wxCloseEvent& event );
        double lat1, lon1, lat2, lon2;
        bool error_found;
        bool dbg;

		wxString     m_gpx_path;	

		wxString    TC[12][3];  //Date.Time, Height, Units, HW.LW
		float       tcv[26];
		
		bool        btc_valid;
		int         m_corr_mins;
		wxString    m_stz;
	
		wxString m_PortName;
		wxString s2time;

};


class VMHData
{
public:
    double latD, lonD;
    wxString lat, lon;
    VMHData *prev, *next; /* doubly linked circular list of positions */
    wxString location;
	wxString name;
	wxString portno;
	wxString M2H;
};

class Position
{
public:
	double latD, lonD;
    wxString lat, lon;
	wxString stat_num, port_num;
	wxString minus_plus[12];
		
	wxString minus_6, minus_5, minus_4, minus_3 ,minus_2, minus_1, zero;
	wxString plus_1, plus_2,  plus_3, plus_4, plus_5, plus_6;
    Position *prev, *next; /* doubly linked circular list of positions */
};

class PortTides
{
public:

	wxString m_portID, m_portName, m_IDX;
	double m_spRange, m_npRange;

};


class StandardPort
{
public:
	wxString PORT_NUMBER,PORT_NAME,MEAN_SPRING_RANGE,MEAN_NEAP_RANGE,EXTRA,IDX;
};



class CalendarDialog: public wxDialog
{
public:
 
	CalendarDialog ( wxWindow * parent, wxWindowID id, const wxString & title,
	              const wxPoint & pos = wxDefaultPosition,
	              const wxSize & size = wxDefaultSize,
	              long style = wxDEFAULT_DIALOG_STYLE );
 
	//wxTextCtrl * dialogText;
	wxCalendarCtrl* dialogCalendar; 

	wxStaticText *m_staticText; 
	wxTimeTextCtrl *_timeText;
	wxSpinButton *_spinCtrl;
	wxString GetText();
    void spinUp(wxSpinEvent& event);
	void spinDown(wxSpinEvent& event);
private:
 
	void OnOk( wxCommandEvent & event );

};

class GetPortDialog: public wxDialog
{
public:
 
	GetPortDialog ( wxWindow * parent, wxWindowID id, const wxString & title,
	              const wxPoint & pos = wxDefaultPosition,
	              const wxSize & size = wxDefaultSize,
	              long style = wxDEFAULT_DIALOG_STYLE );
 
	wxListView * dialogText;
	wxString GetText();
 
private:
 
	void OnOk( wxCommandEvent & event );

};

#endif

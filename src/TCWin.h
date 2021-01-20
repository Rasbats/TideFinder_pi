/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#ifndef __TCWIN_H__
#define __TCWIN_H__

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers
#include <vector>
#include "TideFindergui_impl.h"
#include <wx/dialog.h>
#include <wx/datetime.h>
#include <wx/timer.h>
#include <wx/list.h>
#include "tcmgr.h"
#include "wx/event.h"

class wxCommandEvent;
class wxCloseEvent;
class wxTextCtrl;
class wxButton;
class wxListBox;

class Dlg;

//using namespace std;

WX_DECLARE_LIST(wxPoint, SplineList);           // for spline curve points

class TCWin: public wxDialog
{
public:
      TCWin(wxWindow * parent, int x, int y, int PortNo, wxString PortName, int graphday_00_s, wxDateTime graphDayD, int offset, double lat, wxString myUnits);
      ~TCWin();

      void OnSize(wxSizeEvent& event);
      void OnPaint(wxPaintEvent& event);
      void MouseEvent(wxMouseEvent& event);
	  void OnTCWinPopupTimerEvent(wxTimerEvent& event);
      void OKEvent(wxCommandEvent& event);
      void NXEvent(wxCommandEvent& event);
      void PREvent(wxCommandEvent& event);
      void OnCloseWindow(wxCloseEvent& event);

      void Resize(void);

      void RePosition(void);
	  void Test( );
	  wxString myTCPortNo;

private:

	Dlg  *myDlg;
	//TCMgr *ptcmgr;
	//IDX_entry *pIDX;
	wxColour c_black1;
	wxColour c_grey;
	wxColour c_grey2;
	wxColour c_blue;
	int m_passPort;
	wxString m_passName;
	int  m_passStation;
	wxDateTime m_passGraphDay;
	int m_passOffset;
	double m_passLat;
	wxString m_passUnits;
    wxStaticText  *m_ptextctrl;

    int           curs_x;
    int           curs_y;
    int          m_plot_type;


      wxButton    *OK_button;
      wxButton    *NX_button;
      wxButton    *PR_button;

      int         im;
      int         ib;
      int         it;
      int         val_off;
      wxRect      m_graph_rect;


      float       tcv[26];
	  wxListBox  *m_tList ;
      bool        btc_valid;
      wxWindow   *pParent;
      int         m_corr_mins;
      wxString    m_stz;
      int         m_t_graphday_00_at_station;
      wxDateTime  m_graphday;
      int         m_plot_y_offset;

      SplineList  m_sList;

      wxFont *pSFont;
      wxFont *pSMFont;
      wxFont *pMFont;
      wxFont *pLFont;
	  wxFont *pVLFont;

      wxPen *pblack_1;
      wxPen *pblack_2;
      wxPen *pblack_3;
      wxPen *pred_2;
      wxBrush *pltgray;
      wxBrush *pltgray2;


DECLARE_EVENT_TABLE()
};


#endif

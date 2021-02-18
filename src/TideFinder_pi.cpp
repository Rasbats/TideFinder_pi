/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  TideFinder Plugin
 * Author:   Mike Rossiter
 *
 ***************************************************************************
 *   Copyright (C) 2013 by Mike Rossiter                                *
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

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "TideFinder_pi.h"
#include "TideFindergui_impl.h"
#include "TideFindergui.h"
#include "ocpn_plugin.h" 


class TideFinder_pi;

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new TideFinder_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}

//---------------------------------------------------------------------------------------------------------
//
//    TideFinder PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

#include "icons.h"

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

TideFinder_pi::TideFinder_pi(void *ppimgr)
	:opencpn_plugin_116(ppimgr)
{
	// Create the PlugIn icons
	initialize_images();

	wxFileName fn;

	wxString path = GetPluginDataDir("TideFinder_pi");
	fn.SetPath(path);
	fn.AppendDir("data");
	fn.SetFullName("tidefinder_panel_icon.png");

	path = fn.GetFullPath();

	wxInitAllImageHandlers();

	wxLogDebug(wxString("Using icon path: ") + path);
	if (!wxImage::CanRead(path)) {
		wxLogDebug("Initiating image handlers.");
		wxInitAllImageHandlers();
	}
	wxImage panelIcon(path);
	if (panelIcon.IsOk())
		m_panelBitmap = wxBitmap(panelIcon);
	else
		wxLogWarning("TideFinder panel icon has NOT been loaded");	

	  m_bShowTideFinder = false;
}

TideFinder_pi::~TideFinder_pi(void)
{
     delete _img_tf_pi;
     delete _img_tf;
     
}

int TideFinder_pi::Init(void)
{
      AddLocaleCatalog( _T("opencpn-TideFinder_pi") );

      // Set some default private member parameters
      m_route_dialog_x = 0;
      m_route_dialog_y = 0;
      ::wxDisplaySize(&m_display_width, &m_display_height);

      //    Get a pointer to the opencpn display canvas, to use as a parent for the POI Manager dialog
      m_parent_window = GetOCPNCanvasWindow();

      //    Get a pointer to the opencpn configuration object
      m_pconfig = GetOCPNConfigObject();

      //    And load the configuration items
      LoadConfig();

#ifdef TIDEFINDER_USE_SVG
	  m_leftclick_tool_id = InsertPlugInToolSVG(_T("TideFinder"), _svg_tidefinder, _svg_tidefinder, _svg_tidefinder_toggled,
		  wxITEM_CHECK, _("Tide Finder"), _T(""), NULL, TIDEFINDER_TOOL_POSITION, 0, this);

#else
	  m_leftclick_tool_id = InsertPlugInTool(_T(""), _img_tf, _img_tf, wxITEM_NORMAL,
		  _("Tide Finder"), _T(""), NULL,
		  TIDEFINDER_TOOL_POSITION, 0, this);
#endif
      
	wxMenu dummy_menu;
    m_position_menu_id = AddCanvasContextMenuItem
          (new wxMenuItem(&dummy_menu, -1, _("Tide Finder Position")), this );
    SetCanvasContextMenuItemViz(m_position_menu_id, true);

	
	m_pDialog = NULL;

      return (WANTS_OVERLAY_CALLBACK |
              WANTS_OPENGL_OVERLAY_CALLBACK |		  
		      WANTS_CURSOR_LATLON      |
              WANTS_TOOLBAR_CALLBACK   |
              INSTALLS_TOOLBAR_TOOL    |                        
              WANTS_CONFIG             |
			  WANTS_PLUGIN_MESSAGING
           );
}

bool TideFinder_pi::DeInit(void)
{
      //    Record the dialog position
      if (NULL != m_pDialog)
      {
            //Capture dialog position
            wxPoint p = m_pDialog->GetPosition();
            SetCalculatorDialogX(p.x);
            SetCalculatorDialogY(p.y);
            m_pDialog->Close();
            delete m_pDialog;
            m_pDialog = NULL;

			m_bShowTideFinder = false;
			SetToolbarItemState( m_leftclick_tool_id, m_bShowTideFinder );

      }	
    
    SaveConfig();

    RequestRefresh(m_parent_window); // refresh mainn window

    return true;
}

int TideFinder_pi::GetAPIVersionMajor()
{
	return atoi(API_VERSION);
}

int TideFinder_pi::GetAPIVersionMinor()
{
	std::string v(API_VERSION);
	size_t dotpos = v.find('.');
	return atoi(v.substr(dotpos + 1).c_str());
}

int TideFinder_pi::GetPlugInVersionMajor()
{
      return PLUGIN_VERSION_MAJOR;
}

int TideFinder_pi::GetPlugInVersionMinor()
{
      return PLUGIN_VERSION_MINOR;
}

wxBitmap *TideFinder_pi::GetPlugInBitmap()
{
	return &m_panelBitmap;
}

wxString TideFinder_pi::GetCommonName()
{
      return _("Tide Finder");
}


wxString TideFinder_pi::GetShortDescription()
{
      return _("Tide Finder Plugin");
}

wxString TideFinder_pi::GetLongDescription()
{
      return _("Allows tidal data to be found beyond today");
}

int TideFinder_pi::GetToolbarToolCount(void)
{
      return 1;
}

void TideFinder_pi::SetColorScheme(PI_ColorScheme cs)
{
      if (NULL == m_pDialog)
            return;

      DimeWindow(m_pDialog);
}

void TideFinder_pi::OnToolbarToolCallback(int id)
{
    
	if(NULL == m_pDialog)
      {
            m_pDialog = new Dlg(m_parent_window);
            m_pDialog->plugin = this;
            m_pDialog->Move(wxPoint(m_route_dialog_x, m_route_dialog_y));
      }

	  m_pDialog->Fit();
	  //Toggle 
	  m_bShowTideFinder = !m_bShowTideFinder;	  

      //    Toggle dialog? 
      if(m_bShowTideFinder) {
          m_pDialog->Show();         
      } else
          m_pDialog->Hide();
     
      // Toggle is handled by the toolbar but we must keep plugin manager b_toggle updated
      // to actual status to ensure correct status upon toolbar rebuild
      SetToolbarItemState( m_leftclick_tool_id, m_bShowTideFinder );

      RequestRefresh(m_parent_window); // refresh main window
}

void TideFinder_pi::OnContextMenuItemCallback(int id)
{
    if(!plugin)
        return;

    if(id == m_position_menu_id)
	   
	    m_cursor_lat = GetCursorLat();
		m_cursor_lon = GetCursorLon();
        
		wxString myLat = wxString::Format(wxT("%5.2f"), (double) m_cursor_lat); 

		 if(m_bShowTideFinder) {
          m_pDialog->OnContextMenu(m_cursor_lat, m_cursor_lon);
		 }

}

void TideFinder_pi::AddPosition(double lat, double lon)
{
	wxMessageBox(wxT("From the menu"));
}

bool TideFinder_pi::LoadConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if(pConf)
      {
          
		  pConf->SetPath ( _T( "/Settings/TF_pi" ) );
			
		   pConf->Read ( _T( "ShowTideFinderIcon" ), &m_bTideFinderShowIcon, 1 );
           
            m_route_dialog_x =  pConf->Read ( _T ( "DialogPosX" ), 20L );
            m_route_dialog_y =  pConf->Read ( _T ( "DialogPosY" ), 20L );
         
            if((m_route_dialog_x < 0) || (m_route_dialog_x > m_display_width))
                  m_route_dialog_x = 5;
            if((m_route_dialog_y < 0) || (m_route_dialog_y > m_display_height))
                  m_route_dialog_y = 5;
            return true;
      }
      else
            return false;
}

bool TideFinder_pi::SaveConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if(pConf)
      {
            pConf->SetPath ( _T ( "/Settings/TF_pi" ) );
			pConf->Write ( _T ( "ShowTideFinderIcon" ), m_bTideFinderShowIcon );
          
            pConf->Write ( _T ( "DialogPosX" ),   m_route_dialog_x );
            pConf->Write ( _T ( "DialogPosY" ),   m_route_dialog_y );
            
            return true;
      }
      else
            return false;
}

void TideFinder_pi::ShowPreferencesDialog( wxWindow* parent )
{
      //CfgDlg *dialog = new CfgDlg( parent, wxID_ANY, _("Route Preferences"), wxPoint( m_route_dialog_x, m_route_dialog_y), wxDefaultSize, wxDEFAULT_DIALOG_STYLE );
      //dialog->Fit();
      //wxColour cl;
      //DimeWindow(dialog);
      
      //if(dialog->ShowModal() == wxID_OK)
      //{           
        //    SaveConfig();
      //}
      //delete dialog;
}

void TideFinder_pi::OnTideFinderDialogClose()
{
    m_bShowTideFinder = false;
    SetToolbarItemState( m_leftclick_tool_id, m_bShowTideFinder );
    m_pDialog->Hide();
    SaveConfig();

    RequestRefresh(m_parent_window); // refresh main window

}

void TideFinder_pi::SetCursorLatLon(double lat, double lon)
{
        m_cursor_lat=lat;
        m_cursor_lon=lon;
}

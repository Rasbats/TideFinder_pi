# ~~~
# Summary:      Local, non-generic plugin setup
# Copyright (c) 2020-2021 Mike Rossiter
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.


# -------- Options ----------

set(OCPN_TEST_REPO
    "opencpn/tidefinder-alpha"
    CACHE STRING "Default repository for untagged builds"
)
set(OCPN_BETA_REPO
    "opencpn/tidefinder-beta"
    CACHE STRING
    "Default repository for tagged builds matching 'beta'"
)
set(OCPN_RELEASE_REPO
    "opencpn/tidefinder-prod"
    CACHE STRING
    "Default repository for tagged builds not matching 'beta'"
)

option(TIDEFINDER_USE_SVG "Use SVG graphics" ON)

#
#
# -------  Plugin setup --------
#
set(PKG_NAME TideFinder_pi)
set(PKG_VERSION  1.4.3)
set(PKG_PRERELEASE "")  # Empty, or a tag like 'beta'

set(DISPLAY_NAME TideFinder)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME TideFinder) # As of GetCommonName() in plugin API
set(PKG_SUMMARY "Predict future tides")
set(PKG_DESCRIPTION [=[
Extends the tidal predictions of OpenCPN using a calendar to allow 
easier future predictions.
]=])

set(PKG_AUTHOR "Mike Rossiter")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/Rasbats/TideFinder_pi)
set(PKG_INFO_URL https://opencpn.org/OpenCPN/plugins/TideFinder.html)

set(SRC
    src/TideFinder_pi.h
            src/TideFinder_pi.cpp
            src/icons.h
            src/icons.cpp
            src/TideFindergui.h
            src/TideFindergui.cpp
            src/TideFindergui_impl.cpp
            src/TideFindergui_impl.h
            src/NavFunc.h
            src/NavFunc.cpp
            src/tcmgr.cpp
            src/tcmgr.h
            src/TCWin.cpp
            src/TCWin.h
            src/timectrl.h
            src/timectrl.cpp
)

set(PKG_API_LIB api-16)  #  A directory in libs/ e. g., api-17 or api-16

macro(late_init)
  # Perform initialization after the PACKAGE_NAME library, compilers
  # and ocpn::api is available.
  if (TIDEFINDER_USE_SVG)
    target_compile_definitions(${PACKAGE_NAME} PUBLIC TIDEFINDER_USE_SVG)
  endif ()
endmacro ()

macro(add_plugin_libraries)
  # Add libraries required by this plugin
  add_subdirectory("libs/tinyxml")
  target_link_libraries(${PACKAGE_NAME} ocpn::tinyxml)

  add_subdirectory("libs/wxJSON")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)

  add_subdirectory("libs/plugingl")
  target_link_libraries(${PACKAGE_NAME} ocpn::plugingl)

  add_subdirectory("libs/jsoncpp")
  target_link_libraries(${PACKAGE_NAME} ocpn::jsoncpp)
endmacro ()

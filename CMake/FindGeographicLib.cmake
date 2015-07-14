#ckwg +4
# Copyright 2010-2014 by Kitware, Inc. All Rights Reserved. Please refer to
# KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
# Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.

# The following variables will guide the build:
#
# GeographicLib_ROOT        - Set to the install prefix of the PROJ library
#
# The following variables will be set:
#
# GeographicLib_FOUND       - Set to true if GeographicLib can be found
# GeographicLib_INCLUDE_DIR - The path to the GeographicLib header files
# GeographicLib_LIBRARY     - The full path to the GeographicLib library

if( Geographiclib_DIR )
  find_package( GeographicLib NO_MODULE )
elseif( NOT GeographicLib_FOUND )
  include(CommonFindMacros)

  setup_find_root_context(GeographicLib)
  find_path( GeographicLib_INCLUDE_DIR GeographicLib/GeoCoords.hpp
    ${GeographicLib_FIND_OPTS})
  find_library( GeographicLib_LIBRARY
    NAMES Geographic GeographicLib Geographic_d GeographicLib_d
    ${GeographicLib_FIND_OPTS})
  restore_find_root_context(GeographicLib)

  include( FindPackageHandleStandardArgs )
  FIND_PACKAGE_HANDLE_STANDARD_ARGS( GeographicLib GeographicLib_INCLUDE_DIR GeographicLib_LIBRARY )
  if( GEOGRAPHICLIB_FOUND )
    set( GeographicLib_FOUND TRUE )
  endif()
endif()

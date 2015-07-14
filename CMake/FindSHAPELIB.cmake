#ckwg +4
# Copyright 2010 2014 by Kitware, Inc. All Rights Reserved. Please refer to
# KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
# Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.

# Locate the system installed SHAPELIB
# The following variables will be set:
#
# SHAPELIB_FOUND       - Set to true if SHAPELIB can be found
# SHAPELIB_INCLUDE_DIR - The path to the SHAPELIB header files
# SHAPELIB_LIBRARY     - The full path to the SHAPELIB library

if( SHAPELIB_DIR )
  find_package( SHAPELIB NO_MODULE )
elseif( NOT SHAPELIB_FOUND )
  include(CommonFindMacros)

  setup_find_root_context(SHAPELIB)
  find_path( SHAPELIB_INCLUDE_DIR shapefil.h PATH_SUFFIXES libshp ${SHAPELIB_FIND_OPTS})
  find_library( SHAPELIB_LIBRARY shp ${SHAPELIB_FIND_OPTS})
  restore_find_root_context(SHAPELIB)
  
  include( FindPackageHandleStandardArgs )
  FIND_PACKAGE_HANDLE_STANDARD_ARGS( SHAPELIB SHAPELIB_INCLUDE_DIR SHAPELIB_LIBRARY )
endif()

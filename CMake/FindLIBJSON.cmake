#ckwg +4
# Copyright 2012-2014 by Kitware, Inc. All Rights Reserved. Please refer to
# KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
# Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.

# Locate the system installed json
#
# The following variables will guide the build:
#
# LIBJSON_ROOT        - Set to the install prefix of the json library
# LIBJSON_LIBNAME     - Name of the installed library (defaults to json)
#
# The following variables will be set:
#
# LIBJSON_FOUND       - Set to true if json can be found
# LIBJSON_INCLUDE_DIR - The path to the json header files
# LIBJSON_LIBRARY     - The full path to the json library

if( LIBJSON_DIR )
  find_package( LIBJSON NO_MODULE )
elseif( NOT LIBJSON_FOUND )
  include(CommonFindMacros)

  if(NOT LIBJSON_LIBNAME)
    set(LIBJSON_LIBNAME json)
  endif()

  setup_find_root_context(LIBJSON)
  find_path(LIBJSON_INCLUDE_DIR ${LIBJSON_LIBNAME}.h
    PATH_SUFFIXES ${LIBJSON_LIBNAME} ${LIBJSON_FIND_OPTS})
  find_library(LIBJSON_LIBRARY ${LIBJSON_LIBNAME} ${LIBJSON_FIND_OPTS})
  restore_find_root_context(LIBJSON)

  include( FindPackageHandleStandardArgs )
  FIND_PACKAGE_HANDLE_STANDARD_ARGS( LIBJSON LIBJSON_INCLUDE_DIR LIBJSON_LIBRARY )
endif()

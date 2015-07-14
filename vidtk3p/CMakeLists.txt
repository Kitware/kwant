#ckwg +4
# Copyright 2010-2014 by Kitware, Inc. All Rights Reserved. Please refer to
# KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
# Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.

# KWSys borrowed from CMake
#---------------------------------------------------------------------
# Create the kwsys library for vidtk.
set(KWSYS_NAMESPACE vidtksys)
set(KWSYS_USE_DynamicLoader 1)
set(KWSYS_INSTALL_EXPORT_NAME vidtk)
set(KWSYS_INSTALL_LIB_DIR lib)
set(KWSYS_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/KWSys CACHE INTERNAL "")
add_subdirectory(KWSys)


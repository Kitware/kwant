cmake_minimum_required(VERSION 3.0)

project( KWANT )

include(CMakeDependentOption)
###
# KWANT version
set(KWANT_VERSION_MAJOR 0)
set(KWANT_VERSION_MINOR 1)
set(KWANT_VERSION_PATCH 0)
set(KWANT_VERSION "${KWANT_VERSION_MAJOR}.${KWANT_VERSION_MINOR}.${KWANT_VERSION_PATCH}")

###
# Ask for kwiver build directory
#
set( kwiver_DIR "" CACHE PATH "Path to KWIVER packages" )

###
# project specific directories
#
set(kwant_CMAKE_DIR            "${CMAKE_SOURCE_DIR}/CMake")

# root directories
set(kwant_SOURCE_DIR           "${CMAKE_SOURCE_DIR}")
set(kwant_BINARY_DIR           "${CMAKE_BINARY_DIR}")

#Set CMP0022 to NEW so we error if someone writes older style cmake code
#See http://www.cmake.org/cmake/help/v2.8.12/cmake.html#policy:CMP0022
#for details on CMP0022
if(POLICY CMP0022)
  cmake_policy(SET CMP0022 NEW)
endif()

if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

if( NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY )
  set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${kwant_BINARY_DIR}/lib )
endif()
if( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${kwant_BINARY_DIR}/lib )
endif()
if( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${kwant_BINARY_DIR}/bin )
endif()

###
# Add the CMake directory for CMake modules
#
list(INSERT CMAKE_MODULE_PATH 0  "${kwant_SOURCE_DIR}/CMake"  )

###
# Options and setup
#
OPTION(KWANT_BUILD_SHARED         "Build KWANT components shared or not" TRUE )
set(BUILD_SHARED_LIBS ${KWANT_BUILD_SHARED})

include_directories( ${kwant_SOURCE_DIR} )
include_directories( ${kwant_BINARY_DIR} )

include( kwant-depends )

add_subdirectory( scoring_framework )

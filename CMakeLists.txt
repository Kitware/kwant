cmake_minimum_required( VERSION 2.8.12 )

project( kwant )

# add the CMake directory for CMake modules
set(CMAKE_MODULE_PATH "${kwant_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
include_directories( ${kwant_SOURCE_DIR} )

# vision-tpl can provide many of the external dependencies
find_package(vision-tpl REQUIRED)
# if (vision-tpl_FOUND)
#   include(find_vtpl_package)
# endif()

find_package(VXL REQUIRED)
include_directories(SYSTEM ${VXL_VCL_INCLUDE_DIR})
include_directories(SYSTEM ${VXL_CORE_INCLUDE_DIR})
link_directories(${VXL_LIBRARY_DIR})

include(GitVersionTest)
check_reachable_git_hash(VXL ${VXL_CMAKE_DIR} "e03c56a32e5f52c80d86b503a778210437bb861d")

#Set CMP0022 to NEW so we error if someone writes older style cmake code
#See http://www.cmake.org/cmake/help/v2.8.12/cmake.html#policy:CMP0022
#for details on CMP0022
if(POLICY CMP0022)
  cmake_policy(SET CMP0022 NEW)
endif()

#Set CMP0048 to OLD until we can move min cmake to 3.0
#See http://www.cmake.org/cmake/help/v3.0/policy/CMP0048.html for details
#We need OLD to maintain the old approach to versioning ( if we even use it )
if(POLICY CMP0048)
  cmake_policy(SET CMP0048 OLD)
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

if( WIN32 )
  set(Boost_USE_STATIC_LIBS True)
endif()
find_package( Boost 1.42 REQUIRED COMPONENTS thread filesystem system date_time regex)
add_definitions( -DBOOST_ALL_NO_LIB )
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# we can't build shared libraries on Windows so we leave it off by default;
# we haven't set up DLL exports or anything like that
option(BUILD_SHARED_LIBS "Build with shared libraries." OFF)

option( KWANT_ENABLE_SHAPELIB "Enable Shapelib dependent code" OFF)


find_package( TinyXML REQUIRED )
include_directories( SYSTEM ${TinyXML_INCLUDE_DIR} )

find_package(LIBJSON REQUIRED)
include_directories ( SYSTEM ${LIBJSON_INCLUDE_DIR} )

add_subdirectory( vidtk3p )
add_subdirectory( logger )

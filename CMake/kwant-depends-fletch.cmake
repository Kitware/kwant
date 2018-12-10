#The following required packages are provided by fletch

find_package( fletch REQUIRED )
list(APPEND CMAKE_PREFIX_PATH "${fletch_ROOT}")

find_package( TinyXML REQUIRED )
include_directories( SYSTEM ${TinyXML_INCLUDE_DIR} )

find_package( LIBJSON REQUIRED )
include_directories( SYSTEM ${LIBJSON_INCLUDE_DIR} )

find_package(Boost 1.55 REQUIRED
  COMPONENTS
    date_time
    system
    )
add_definitions(-DBOOST_ALL_NO_LIB)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package( VXL REQUIRED )
include(${VXL_CMAKE_DIR}/UseVXL.cmake)
include_directories( SYSTEM ${VXL_CORE_INCLUDE_DIR} )
include_directories( SYSTEM ${VXL_VCL_INCLUDE_DIR} )
include_directories( SYSTEM ${VXL_RPL_INCLUDE_DIR} )
link_directories( ${VXL_LIBRARY_DIR} )


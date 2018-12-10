find_package( kwiver REQUIRED )

if( NOT ";${KWIVER_LIBRARIES};" MATCHES ";track_oracle;" )
  message( FATAL_ERROR "Kwant requires that kwiver be built with track_oracle enabled." )
endif()

include_directories( SYSTEM ${KWIVER_INCLUDE_DIRS} )
link_directories("${KWIVER_LIBRARY_DIRS}")
include(kwiver-cmake-future)
include(kwiver-utils)
include(kwiver-flags)
include(kwiver-configcheck)

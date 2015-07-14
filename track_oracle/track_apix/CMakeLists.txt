#
# APIX tracks
#

find_package(SHAPELIB REQUIRED)
include_directories( SYSTEM ${SHAPELIB_INCLUDE_DIR} )

find_package(GeographicLib REQUIRED)
include_directories(SYSTEM ${GeographicLib_INCLUDE_DIR})

add_library( track_apix
             track_apix.h
             track_apix_writer.h        track_apix_writer.cxx
             file_format_apix.h         file_format_apix.cxx
)

set_target_properties( track_apix PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_apix
  PUBLIC track_oracle track_oracle_format_base data_terms
  PRIVATE vidtk_geographic ${SHAPELIB_LIBRARY} ${Boost_DATE_TIME_LIBRARY} vul
  )


install( TARGETS track_apix EXPORT vidtk
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
export( TARGETS track_apix APPEND FILE "${VIDTK_EXPORT_FILE}")

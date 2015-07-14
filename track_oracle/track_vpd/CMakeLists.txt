#
# VIRAT Public Data track / events
#

add_library( track_vpd
             track_vpd_track.h    track_vpd_track.cxx
             track_vpd_event.h    track_vpd_event.cxx
             file_format_vpd_track.h    file_format_vpd_track.cxx
             file_format_vpd_event.h    file_format_vpd_event.cxx )
set_target_properties( track_vpd PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_vpd
  PUBLIC track_oracle track_oracle_format_base vgl
  PRIVATE vidtk_logger vul
  )
install( TARGETS track_vpd
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)


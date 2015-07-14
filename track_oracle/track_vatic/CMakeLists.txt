#
# vatic tracks
#

add_library( track_vatic
             track_vatic.h
             file_format_vatic.h        file_format_vatic.cxx )
set_target_properties( track_vatic PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_vatic PUBLIC track_oracle track_oracle_format_base vgl vidtk_logger )
install( TARGETS track_vatic
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

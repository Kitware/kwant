#
# kst tracks
#

add_library( track_kst
             track_kst.h
             file_format_kst.h    file_format_kst.cxx )
set_target_properties( track_kst PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_kst
  PUBLIC track_oracle track_oracle_format_base vgl data_terms
  PRIVATE vidtk_logger vul
  INTERFACE
  )
install( TARGETS track_kst
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

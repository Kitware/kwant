#
# kw18 tracks
#

add_library( track_kw18
             track_kw18.h
             file_format_kw18.h         file_format_kw18.cxx
             )
set_target_properties( track_kw18 PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_kw18
  PUBLIC track_oracle data_terms
  PRIVATE vidtk_logger vul vgl
  INTERFACE track_oracle_format_base
  )

install( TARGETS track_kw18
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)


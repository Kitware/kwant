#
# kwiver tracks
#

add_library( track_kwiver
             track_kwiver.h
             file_format_kwiver.h   file_format_kwiver.cxx
             )
set_target_properties( track_kwiver PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_kwiver
  PUBLIC track_oracle track_oracle_format_base
  PRIVATE ${TinyXML_LIBRARY} track_oracle_tokenizers logging_map
  )
install( TARGETS track_kwiver
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

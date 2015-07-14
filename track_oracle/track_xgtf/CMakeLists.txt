#
# xgtf tracks
#

add_library( track_xgtf
             track_xgtf.h
             file_format_xgtf.h         file_format_xgtf.cxx )
set_target_properties( track_xgtf PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_xgtf
  PUBLIC track_oracle track_oracle_format_base vgl
  PRIVATE track_oracle_tokenizers logging_map vul ${TinyXML_LIBRARY}
  )
install( TARGETS track_xgtf
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

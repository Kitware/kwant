#
# comms XML tracks
#

add_library( track_comms_xml
             track_comms_xml.h
             file_format_comms_xml.h        file_format_comms_xml.cxx )
set_target_properties( track_comms_xml PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_comms_xml
  PUBLIC track_oracle track_oracle_format_base vgl
  PRIVATE vidtk_logger ${TinyXML_LIBRARY} track_oracle_tokenizers
  )
install( TARGETS track_comms_xml
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

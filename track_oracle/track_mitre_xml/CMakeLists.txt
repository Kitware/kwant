#
# mitre XML tracks
#

add_library( track_mitre_xml
             track_mitre_xml.h
             file_format_mitre_xml.h         file_format_mitre_xml.cxx )
set_target_properties( track_mitre_xml PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_mitre_xml
  PUBLIC track_oracle track_oracle_format_base vgl
  PRIVATE track_oracle_tokenizers vidtk_logger ${TinyXML_LIBRARY}
  )
install( TARGETS track_mitre_xml
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

#
# kwxml tracks
#

add_library( track_kwxml
             track_kwxml.h
             file_format_kwxml.h    file_format_kwxml.cxx )
set_target_properties( track_kwxml PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_kwxml
  PUBLIC track_oracle track_oracle_format_base vgl data_terms
  PRIVATE logging_map scoring_aries_interface vidtk_logger track_oracle_tokenizers ${TinyXML_LIBRARY}
  INTERFACE
  )
install( TARGETS track_kwxml
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

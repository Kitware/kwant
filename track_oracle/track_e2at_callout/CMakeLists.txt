#
# e2at callouts
#

add_library( track_e2at_callout
             track_e2at_callout.h
             file_format_e2at_callout.h   file_format_e2at_callout.cxx )
set_target_properties( track_e2at_callout PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_e2at_callout
  PUBLIC track_oracle track_oracle_format_base
  PRIVATE data_terms track_oracle_tokenizers vidtk_logger vul
  )
install( TARGETS track_e2at_callout
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

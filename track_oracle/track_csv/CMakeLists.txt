#
# Generic CSV tracks
#

add_library( track_csv
             track_csv.h
             file_format_csv.h  file_format_csv.cxx )
set_target_properties( track_csv PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_csv
  PUBLIC track_oracle_format_base
  PRIVATE track_oracle data_terms vidtk_logger scorable_mgrs_data_term track_oracle_tokenizers vul
  )

install( TARGETS track_csv
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

#
# kwe track filter
#

add_library( track_filter_kwe
             track_filter_kwe.h         track_filter_kwe.cxx
             )
set_target_properties( track_filter_kwe PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_filter_kwe
  PUBLIC track_oracle_event_adapter
  PRIVATE track_oracle scoring_aries_interface vidtk_logger logging_map vul
  )
install( TARGETS track_filter_kwe
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

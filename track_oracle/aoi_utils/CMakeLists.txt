add_library( track_oracle_aoi_utils
  aoi_utils.h        aoi_utils.cxx
)

set_target_properties( track_oracle_aoi_utils PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_oracle_aoi_utils
  PRIVATE track_oracle track_scorable_mgrs vidtk_logger vgl ${Boost_REGEX_LIBRARY}
)

install( TARGETS track_oracle_aoi_utils
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

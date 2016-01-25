set( kwiver_geographic_public_headers
  geo_coords.h
)

set( kwiver_geographic_sources
  geo_coords.cxx
)

kwiver_install_headers(
  ${kwiver_geographic_public_headers}
  SUBDIR kwant/geographic
)

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_geographic_export.h
  SUBDIR kwant/geographic
  NOPATH
)

kwiver_add_library(
  kwiver_geographic
  ${kwiver_geographic_public_headers}
  ${kwiver_geographic_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_geographic_export.h
)

target_link_libraries( kwiver_geographic
  PRIVATE              ${GeographicLib_LIBRARY}
)


add_subdirectory(tests)
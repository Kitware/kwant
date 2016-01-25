include_directories("${CMAKE_CURRENT_BINARY_DIR}")

set( kwant_logging_map_public_headers
  logging_map.h
)

set( kwant_logging_map_sources
  logging_map.cxx
)

kwiver_install_headers(
  ${kwant_logging_map_public_headers}
  SUBDIR          kwant/track_oracle/utils
)

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/logging_map_export.h
  SUBDIR          kwant/track_oracle/utils
  NOPATH
)

kwiver_add_library( logging_map
  ${kwant_logging_map_public_headers}
  ${kwant_logging_map_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/logging_map_export.h
)

target_link_libraries( logging_map
  PUBLIC      vital_logger
)


set( kwant_tokenizers_public_headers
  tokenizers.h
)

set( kwant_tokenizers_sources
  tokenizers.cxx
)

kwiver_install_headers(
  ${kwant_tokenizers_public_headers}
  SUBDIR         kwant/utils
)

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/track_oracle_tokenizers_export.h
  SUBDIR         kwant/utils
  NOPATH
)

kwiver_add_library( track_oracle_tokenizers
  ${kwant_tokenizers_public_headers}
  ${kwant_tokenizers_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/track_oracle_tokenizers_export.h
)

target_link_libraries( track_oracle_tokenizers
  PRIVATE         vital_logger
)


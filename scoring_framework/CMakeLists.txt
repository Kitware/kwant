OPTION(KWANT_ENABLE_MGRS  "Enable track_oracle's lat/lon <-> MGRS capabilities (disable until further notice)" OFF )

if (KWANT_ENABLE_MGRS)
  message( FATAL_ERROR "Building the MGRS code is blocked until lat/lon <-> MGRS capabilities are available" )
  #find_package( GeographicLib REQUIRED )
  #include_directories ( ${GeographicLib_INCLUDE_DIR} )
  #add_definitions(-DKWANT_ENABLE_MGRS)
endif()

########################################
# timestamp utilities
########################################
set( timestamp_utilities_public_headers
  timestamp_utilities.h
)

set( timestamp_utilities_sources
  timestamp_utilities.cxx
)

kwiver_install_headers(
  ${timestamp_utilities_public_headers}
  SUBDIR        scoring_framework
)

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/timestamp_utilities_export.h
  SUBDIR        scoring_framework
  NOPATH
)

kwiver_add_library( timestamp_utilities
  ${timestamp_utilities_public_headers}
  ${timestamp_utilities_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/timestamp_utilities_export.h
)

target_link_libraries( timestamp_utilities
  PUBLIC               track_oracle
  PRIVATE              vital_logger
                       vul
                       ${Boost_DATE_TIME_LIBRARY}
)

########################################
# Default
########################################

set( score_core_public_headers
  score_core.h
  phase1_parameters.h
  quickfilter_box.h
  score_phase1.h
  matching_args_type.h
  time_window_filter.h
  virat_scenario_utilities.h
)

set( score_core_sources
  phase1_parameters.cxx
  quickfilter_box.cxx
  score_phase1.cxx
  matching_args_type.cxx
  time_window_filter.cxx
  virat_scenario_utilities.cxx
)

kwiver_install_headers(
  ${score_core_public_headers}
  SUBDIR        scoring_framework
)

kwiver_add_library( score_core
  ${score_core_public_headers}
  ${score_core_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/score_core_export.h
)

target_link_libraries( score_core
  PUBLIC               track_oracle
                       ${TRACK_ORACLE_SCORABLE_MGRS_LIBRARY}
                       timestamp_utilities
                       vul
                       vgl
                       data_terms
                       scoring_aries_interface
  PRIVATE              track_oracle_tokenizers
                       vibrant_descriptors
                       vital_logger
                       ${Boost_DATE_TIME_LIBRARY}
                       ${TinyXML_LIBRARY}
)

set( score_tracks_loader_public_headers
  score_tracks_loader.h
)

set( score_tracks_loader_sources
  score_tracks_loader.cxx
)

kwiver_install_headers(
  ${score_tracks_loader_public_headers}
  SUBDIR        scoring_framework
)

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/score_tracks_loader_export.h
  SUBDIR        scoring_framework
  NOPATH
)

kwiver_add_library( score_tracks_loader
  ${score_tracks_loader_public_headers}
  ${score_tracks_loader_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/score_tracks_loader_export.h
)

target_link_libraries( score_tracks_loader
  PUBLIC               score_core
                       vul
                       track_oracle_file_formats
                       track_oracle_format_base
                       track_kw18
                       track_xgtf
                       track_kwxml
                       track_vpd
                       track_kst
                       track_comms_xml
  PRIVATE              vital_logger
)

########################################
# HADWAV
########################################

set( score_tracks_hadwav_public_headers
  score_tracks_hadwav.h
  score_phase2_hadwav.h
  score_phase3_hadwav.h
  score_tracks_hadwav.h
)

set( score_tracks_hadwav_sources
  score_phase2_hadwav.cxx
  score_phase3_hadwav.cxx
)

kwiver_install_headers(
  ${score_tracks_hadwav_public_headers}
  SUBDIR        scoring_framework
)

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/score_tracks_hadwav_export.h
  SUBDIR        scoring_framework
  NOPATH
)

kwiver_add_library( score_tracks_hadwav
  ${score_tracks_hadwav_public_headers}
  ${score_tracks_hadwav_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/score_tracks_hadwav_export.h
)

target_link_libraries(score_tracks_hadwav
  PUBLIC              score_core
  PRIVATE             track_oracle
                      vital_logger
)

########################################
# Scoring
########################################

if (fletch_ENABLED_libjson)
  set(scoring_json_src)
  set(json_lib ${LIBJSON_LIBRARY})

  kwiver_add_executable( score_tracks score_tracks.cxx)
  target_link_libraries( score_tracks
                         vital_logger
                         score_tracks_loader
                         score_tracks_hadwav
                         track_oracle
                         track_oracle_file_formats
                         timestamp_utilities
                         ${json_lib} )
endif()

kwiver_add_executable( score_events score_events.cxx )
target_link_libraries( score_events
                       vital_logger
                       logging_map
                       data_terms
                       score_tracks_loader
                       score_tracks_hadwav
                       scoring_aries_interface
                       timestamp_utilities track_oracle
                       track_oracle_format_base
                       track_oracle_file_formats
                       vibrant_descriptors
                       vul
                       track_filter_kpf_activity
                       kwiver_algo_kpf
                       kpf_yaml
                       kwiversys
                       kpf_utils
                       ${YAML_CPP_LIBRARIES}
                       ${Boost_SYSTEM_LIBRARY} )

########################################
# Helper code for testing
########################################

set( track_synthesizer_public_headers
  track_synthesizer.h
)

set( track_synthesizer_sources
  track_synthesizer.cxx
)

kwiver_install_headers(
  ${track_synthesizer_public_headers}
  SUBDIR        scoring_framework
)

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/track_synthesizer_export.h
  SUBDIR        scoring_framework
  NOPATH
)

kwiver_add_library( track_synthesizer
  ${track_synthesizer_public_headers}
  ${track_synthesizer_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/track_synthesizer_export.h
)
target_link_libraries( track_synthesizer
  PUBLIC               track_oracle
                       score_core
  PRIVATE              vital_logger
                       vnl
)

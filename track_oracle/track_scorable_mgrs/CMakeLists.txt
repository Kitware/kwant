
#
# the scorable MGRS schema.  Doesn't really exist as a separate track
# file, but included here to simplify the template instances.
#

##
## The scorable_mgrs object
##

set( kwant_scorable_mgrs_public_headers
  scorable_mgrs.h
)

set( kwant_scorable_mgrs_sources
  scorable_mgrs.cxx
)

kwiver_install_headers(
  ${kwant_scorable_mgrs_public_headers}
  SUBDIR       kwant/track_oracle/scorable_mgrs
)

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/scorable_mgrs_export.h
  SUBDIR       kwant/track_oracle/scorable_mgrs
  NOPATH
)

kwiver_add_library( scorable_mgrs
  ${kwant_scorable_mgrs_public_headers}
  ${kwant_scorable_mgrs_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/scorable_mgrs_export.h
)

target_link_libraries( scorable_mgrs
  PUBLIC          kwiver_geographic
  PRIVATE         vital_logger
)

##
## The corresponding data term
##


set( kwant_scorable_mgrs_data_term_public_headers
  scorable_mgrs_data_term.h
)

set( kwant_scorable_mgrs_data_term_sources
  scorable_mgrs_data_term.cxx
  scorable_mgrs_data_term_instance.cxx
)

kwiver_install_headers(
  ${kwant_scorable_mgrs_data_term_public_headers}
  SUBDIR       kwant/track_oracle/scorable_mgrs
)

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/scorable_mgrs_data_term_export.h
  SUBDIR       kwant/track_oracle/scorable_mgrs
  NOPATH
)

kwiver_add_library( scorable_mgrs_data_term
  ${kwant_scorable_mgrs_public_headers}
  ${kwant_scorable_mgrs_data_term_public_headers}
  ${kwant_scorable_mgrs_data_term_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/scorable_mgrs_data_term_export.h
)

target_link_libraries( scorable_mgrs_data_term
  PUBLIC         track_oracle scorable_mgrs
  PRIVATE        vital_logger ${TinyXML_LIBRARY}
)

##
## the track object
##

set( kwant_track_scorable_mgrs_public_headers
  track_scorable_mgrs.h
)

set( kwant_track_scorable_mgrs_sources
  track_scorable_mgrs.cxx
)

kwiver_install_headers(
  ${kwant_track_scorable_mgrs_public_headers}
  SUBDIR       kwant/track_oracle/scorable_mgrs
)

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/track_scorable_mgrs_export.h
  SUBDIR       kwant/track_oracle/scorable_mgrs
  NOPATH
)

kwiver_add_library( track_scorable_mgrs
  ${kwant_track_scorable_mgrs_public_headers}
  ${kwant_track_scorable_mgrs_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/track_scorable_mgrs_export.h
)

if (KWANT_ENABLE_SHAPELIB)
    set(TRACK_APIX_LIB track_apix)
endif()

target_link_libraries( track_scorable_mgrs
  PUBLIC            track_oracle scorable_mgrs scorable_mgrs_data_term
  PRIVATE           track_kw18 vital_logger ${TRACK_APIX_LIB}
)

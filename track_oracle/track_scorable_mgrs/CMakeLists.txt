#
# the scorable MGRS schema.  Doesn't really exist as a separate track
# file, but included here to simplify the template instances.
#

add_library( scorable_mgrs
             scorable_mgrs.h      scorable_mgrs.cxx )
set_target_properties( scorable_mgrs PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( scorable_mgrs
  PUBLIC vidtk_geographic
  PRIVATE vidtk_logger )

install( TARGETS scorable_mgrs
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

add_library( scorable_mgrs_data_term
             scorable_mgrs.h
             scorable_mgrs_data_term.h scorable_mgrs_data_term.cxx
             scorable_mgrs_data_term_instance.cxx )
set_target_properties( scorable_mgrs_data_term PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)
target_link_libraries( scorable_mgrs_data_term
  PUBLIC track_oracle scorable_mgrs
  PRIVATE vidtk_logger ${TinyXML_LIBRARY}
)
install( TARGETS scorable_mgrs_data_term
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
  )


add_library( track_scorable_mgrs
             track_scorable_mgrs.h      track_scorable_mgrs.cxx )
set_target_properties( track_scorable_mgrs PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

if (VIDTK_ENABLE_SHAPELIB)
    set(TRACK_APIX_LIB track_apix)
endif()

target_link_libraries( track_scorable_mgrs
  PUBLIC track_oracle scorable_mgrs scorable_mgrs_data_term
  PRIVATE track_kw18 vidtk_logger ${TRACK_APIX_LIB}
  )

install( TARGETS track_scorable_mgrs
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

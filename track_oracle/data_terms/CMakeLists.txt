#
# track oracle data terms
#

add_library( data_terms
             data_terms_common.h
             data_term_tmp_utils.h
             data_terms.h data_terms.cxx
             data_term_instances.cxx
 )

set_target_properties( data_terms PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( data_terms
  PUBLIC track_oracle ${TinyXML_LIBRARY}
  PRIVATE scoring_aries_interface vidtk_logger ${Boost_SYSTEM_LIBRARY}
  INTERFACE vidtk_utilities_no_process vgl
  )

install( TARGETS data_terms
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

add_library( scoring_aries_interface
             aries_interface.h          aries_interface.cxx )

target_link_libraries( scoring_aries_interface
  PRIVATE vidtk_logger
  )
set_target_properties( scoring_aries_interface PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION} )
install( TARGETS scoring_aries_interface
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib )


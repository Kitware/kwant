###
#
# The following block is related to the utilities_no_process library.
#
###
set( vidtk_utilities_sources
  uuid_able.h uuid_able.cxx
)

set( utilities_no_process_private_links
  )
set( utilities_no_process_public_links
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  )

add_library( vidtk_utilities_no_process ${vidtk_utilities_sources} )
set_target_properties( vidtk_utilities_no_process PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( vidtk_utilities_no_process
  PUBLIC ${utilities_no_process_public_links}
  PRIVATE ${utilities_no_process_private_links}
  )
install( TARGETS vidtk_utilities_no_process
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)


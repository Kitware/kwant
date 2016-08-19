###
# kwant processes

project( kwant_processes )

set( sources
  register_processes.cxx

  score_detection_process.cxx
)

set( private_headers
  score_detection_process.cxx
)

kwiver_private_header_group( ${private_headers} )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

kwiver_add_plugin( kwant_processes
  SUBDIR           sprokit
  SOURCES        ${sources}
                 ${private_headers}
  PRIVATE          sprokit_pipeline
                   kwiversys
                   vital vital_apm vital_logger vital_config
                   track_oracle
                   score_core
                 ${Boost_SYSTEM_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
)

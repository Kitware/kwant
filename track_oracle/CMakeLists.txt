add_subdirectory( aries_interface )

add_library( track_oracle
             track_oracle_api_types.h   track_oracle_api_types.cxx

             element_descriptor.h   element_descriptor.cxx
             element_store_base.h   element_store_base.cxx

             element_store.h     element_store.txx      element_store_instantiation.h

             track_oracle_impl.h track_oracle_impl.cxx  track_oracle_impl.txx

             track_oracle.h     track_oracle.cxx        track_oracle.txx  track_oracle_instantiation.h

             track_field_functor.h   track_oracle_instantiation.h

             track_field_functor_library.h
             track_field_host.h track_field_host.cxx
             track_field_base.h track_field_base.cxx
             track_field.h                              track_field.txx    track_field_instantiation.h
             track_field_io_proxy.h                     track_field_io_proxy.txx

             track_field_output_specializations.h       track_field_output_specializations.cxx
             track_oracle_row_view.h  track_oracle_row_view.cxx        track_oracle_row_view.txx  track_oracle_row_view_instantiation.h

             track_oracle_frame_view.h track_oracle_frame_view.cxx

             track_base_impl.h  track_base_impl.cxx

             track_base.h

             xml_output_common_classes.h

             kwiver_io_base.h
             kwiver_io_base_data_io.h  kwiver_io_base_data_io.cxx
             kwiver_io_helpers.h       kwiver_io_helpers.cxx

             schema_algorithm.h    schema_algorithm.cxx

             file_format_type.h    file_format_type.cxx
             file_format_schema.h  file_format_schema.cxx
             file_format_base.h file_format_base.cxx

             state_flags.h               state_flags.cxx

             descriptors/descriptor_metadata_type.h  descriptors/descriptor_metadata_type.cxx
             descriptors/descriptor_motion_type.h descriptors/descriptor_motion_type.cxx
             descriptors/descriptor_cutic_type.h descriptors/descriptor_cutic_type.cxx
             descriptors/descriptor_overlap_type.h descriptors/descriptor_overlap_type.cxx
             descriptors/descriptor_event_label_type.h descriptors/descriptor_event_label_type.cxx
             descriptors/descriptor_raw_1d_type.h descriptors/descriptor_raw_1d_type.cxx
             descriptors/xml_output_descriptor_classes.h

             templates/track_oracle_instances_old_style.cxx
)
set_target_properties( track_oracle PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( track_oracle
  PUBLIC scoring_aries_interface vidtk_utilities_no_process vnl vgl ${Boost_THREAD_LIBRARY}
  PRIVATE vidtk_logger scorable_mgrs track_oracle_tokenizers vul ${TinyXML_LIBRARY}
  )
install( TARGETS track_oracle
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)

add_library( track_oracle_tokenizers
             csv_tokenizer.h csv_tokenizer.cxx
             split.h         split.cxx
             xml_tokenizer.h xml_tokenizer.cxx
)

target_link_libraries( track_oracle_tokenizers PRIVATE vidtk_logger )

set_target_properties( track_oracle_tokenizers PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

install( TARGETS track_oracle_tokenizers
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

add_library( logging_map
             logging_map.h  logging_map.cxx
             )
target_link_libraries( logging_map PUBLIC vidtk_logger )
set_target_properties( logging_map PROPERTIES VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION} )
install( TARGETS logging_map RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib )


add_library( track_oracle_format_base
             file_format_type.h    file_format_type.cxx
             file_format_base.h    file_format_base.cxx
)
target_link_libraries( track_oracle_format_base
  PUBLIC track_oracle vul
  PRIVATE vidtk_logger
  )
set_target_properties( track_oracle_format_base PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)
install( TARGETS track_oracle_format_base
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION
             lib
)

add_subdirectory( example )

add_subdirectory( track_kw18 )

if (VIDTK_ENABLE_SHAPELIB)
    add_subdirectory( track_apix )
    set(TRACK_ORACLE_APIX_LIBRARY track_apix)
endif()

add_subdirectory( aoi_utils )

add_subdirectory( data_terms )

add_library( track_oracle_event_adapter
  event_adapter.h  event_adapter.cxx )
target_link_libraries( track_oracle_event_adapter
  PUBLIC logging_map data_terms
  PRIVATE track_oracle scoring_aries_interface vidtk_logger vul )
set_target_properties( track_oracle_event_adapter
  PROPERTIES VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION} )
install( TARGETS track_oracle_event_adapter
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib )

  add_subdirectory( track_4676 )
  add_subdirectory( track_comms_xml )
  add_subdirectory( track_mitre_xml )
  add_subdirectory( track_kwxml )
  add_subdirectory( track_xgtf )
add_subdirectory( track_kwiver )

add_subdirectory( track_csv )
add_subdirectory( track_kst )
add_subdirectory( track_vatic )
add_subdirectory( track_vpd )
add_subdirectory( track_scorable_mgrs )
add_subdirectory( track_e2at_callout )
add_subdirectory( track_filter_kwe)

add_library( track_oracle_file_formats
             file_format_schema.h  file_format_schema.cxx
             file_format_manager.h file_format_manager.cxx
             schema_factory.h      schema_factory.cxx )

target_link_libraries( track_oracle_file_formats
  PUBLIC
  track_oracle
  track_kw18
  track_mitre_xml
  track_comms_xml
  track_kwxml
  track_xgtf
  track_kst
  track_vatic
  track_vpd
  track_e2at_callout
  track_4676
  track_csv
  track_kwiver
  track_oracle_tokenizers
  track_filter_kwe
  ${TRACK_ORACLE_APIX_LIBRARY}

  PRIVATE
  ${Boost_THREAD_LIBRARY}
  vul
)

install( TARGETS track_oracle_file_formats 
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)



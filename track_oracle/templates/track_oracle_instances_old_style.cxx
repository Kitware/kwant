/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include <string>
#include <set>
#include <utility>

#include <vgl/vgl_box_2d.h>
#include <vgl/vgl_point_2d.h>
#include <vgl/vgl_point_3d.h>

#include <utilities/timestamp.h>
#include <utilities/uuid_able.h>

#include <track_oracle/file_format_manager.h>
#include <track_oracle/track_scorable_mgrs/scorable_mgrs.h>

#include <track_oracle/descriptors/descriptor_cutic_type.h>
#include <track_oracle/descriptors/descriptor_metadata_type.h>
#include <track_oracle/descriptors/descriptor_motion_type.h>
#include <track_oracle/descriptors/descriptor_overlap_type.h>
#include <track_oracle/descriptors/descriptor_event_label_type.h>
#include <track_oracle/descriptors/descriptor_raw_1d_type.h>

#include <track_oracle/track_oracle_instantiation.h>
#include <track_oracle/track_field_instantiation.h>
#include <track_oracle/track_field_functor_instantiation.h>
#include <track_oracle/track_oracle_row_view_instantiation.h>
#include <track_oracle/element_store_instantiation.h>
#include <track_oracle/kwiver_io_base_instantiation.h>

#define TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(T) \
  TRACK_ORACLE_INSTANCES(T) \
  TRACK_FIELD_INSTANCES_OLD_STYLE_DEFAULT_OUTPUT(T) \
  TRACK_FIELD_FUNCTOR_INSTANCES(T) \
  TRACK_ORACLE_ROW_VIEW_INSTANCES(T) \
  ELEMENT_STORE_INSTANCES(T) \
  KWIVER_IO_BASE_INSTANCES(T)

TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(bool);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(int);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(double);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(unsigned int);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(unsigned long);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(unsigned long long);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(std::string)
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(vgl_box_2d<unsigned>);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(vgl_box_2d<double>);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(vgl_point_2d<double>);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(vgl_point_3d<double>);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(vidtk::uuid_t);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(vidtk::file_format_enum);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(vidtk::scorable_mgrs);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(vidtk::timestamp);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_DEFAULT_OUTPUT(std::vector< vidtk::timestamp >);

/// Shouldn't need to distinguish between these, but VS9 has a bug:
/// http://connect.microsoft.com/VisualStudio/feedback/details/753981

#define TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(T) \
  TRACK_ORACLE_INSTANCES(T)   \
  TRACK_FIELD_INSTANCES_OLD_STYLE_SPECIAL_OUTPUT(T) \
  TRACK_FIELD_FUNCTOR_INSTANCES(T) \
  TRACK_ORACLE_ROW_VIEW_INSTANCES(T) \
  ELEMENT_STORE_INSTANCES(T) \
  KWIVER_IO_BASE_INSTANCES(T)


// passing types with a comma is tricky

#define MACRO_COMMA ,

#define TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT_COMMA(T, T2)     \
  TRACK_ORACLE_INSTANCES(T MACRO_COMMA T2) \
  TRACK_FIELD_INSTANCES_OLD_STYLE_SPECIAL_OUTPUT_COMMA(T, T2)           \
  TRACK_FIELD_INSTANCES_GENERAL(T MACRO_COMMA T2) \
  TRACK_FIELD_FUNCTOR_INSTANCES(T MACRO_COMMA T2) \
  TRACK_ORACLE_ROW_VIEW_INSTANCES(T MACRO_COMMA T2) \
  ELEMENT_STORE_INSTANCES(T MACRO_COMMA T2) \
  KWIVER_IO_BASE_INSTANCES(T MACRO_COMMA T2)


TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(vidtk::frame_handle_list_type);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(vidtk::track_handle_list_type);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(std::vector< unsigned >);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(std::vector< double >);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(std::vector< std::vector< double > >);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(std::vector< std::string >);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(std::set< std::string >);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(descriptor_cutic_type);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(descriptor_metadata_type);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(descriptor_motion_type);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(vidtk::descriptor_overlap_type);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(vidtk::descriptor_event_label_type);
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT(vidtk::descriptor_raw_1d_type);

#ifdef WIN32
TRACK_ORACLE_INSTANCES(std::pair<unsigned MACRO_COMMA unsigned >);
TRACK_FIELD_INSTANCES_OLD_STYLE_SPECIAL_OUTPUT(std::pair<unsigned MACRO_COMMA unsigned>);
TRACK_FIELD_INSTANCES_GENERAL(std::pair<unsigned MACRO_COMMA unsigned>);
TRACK_FIELD_FUNCTOR_INSTANCES(std::pair<unsigned MACRO_COMMA unsigned>);
TRACK_ORACLE_ROW_VIEW_INSTANCES(std::pair<unsigned MACRO_COMMA unsigned>);
ELEMENT_STORE_INSTANCES(std::pair<unsigned MACRO_COMMA unsigned>);
KWIVER_IO_BASE_INSTANCES(std::pair<unsigned MACRO_COMMA unsigned>);
#else
TRACK_ORACLE_INSTANTIATE_OLD_STYLE_SPECIAL_OUTPUT_COMMA(std::pair<unsigned, unsigned>);
#endif

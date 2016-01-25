/*ckwg +5
 * Copyright 2014-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <track_oracle/state_flags.h>
#include <track_oracle/data_terms/data_terms.h>
#include <track_oracle/track_oracle_instantiation.h>
#include <track_oracle/track_field_instantiation.h>
#include <track_oracle/track_field_functor_instantiation.h>
#include <track_oracle/track_oracle_row_view_instantiation.h>
#include <track_oracle/element_store_instantiation.h>
#include <track_oracle/kwiver_io_base_instantiation.h>

#define TRACK_ORACLE_INSTANTIATE_DATA_TERM(T) \
  TRACK_FIELD_INSTANCES_DATA_TERM(T) \
  KWIVER_IO_BASE_INSTANCES(T)

TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::utility::state_flags);

TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::external_id );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::timestamp_usecs );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::frame_number );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::fg_mask_area );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::track_location );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::obj_x );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::obj_y );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::obj_location );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::velocity_x );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::velocity_y );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::bounding_box );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::world_x );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::world_y );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::world_z );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::world_location );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::latitude );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::longitude );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::time_stamp );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::world_gcs );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::track_uuid );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::tracking::track_style );

TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::events::event_id );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::events::event_type );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::events::event_probability );
TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::events::source_track_ids );

TRACK_ORACLE_INSTANTIATE_DATA_TERM( ::kwiver::kwant::dt::virat::descriptor_classifier );

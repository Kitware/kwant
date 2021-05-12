/*ckwg +5
 * Copyright 2010-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_SCORE_CORE_H
#define INCL_SCORE_CORE_H

#include <vital/vital_config.h>
#include <scoring_framework/score_core_export.h>

#include <utility>
#include <vgl/vgl_box_2d.h>
#include <track_oracle/core/track_oracle_core.h>
#include <track_oracle/core/track_base.h>
#include <track_oracle/core/track_field.h>
#include <track_oracle/data_terms/data_terms.h>

// The central scoring framework assumption is: Everything you need to
// know in order to score two sets of tracks can be localized to two
// locations: the track itself and the phase-specific contexts.  For example:
// timestamps and bounding boxes are in the track.  Normalization constants
// and AOI filters and so forth are in the contexts.
//
// This file defines the track-specific knowledge.  As long as your track
// defines these three fields, we can score it.
//

namespace kwiver {
namespace kwant {

namespace kwto = ::kwiver::track_oracle;


typedef std::pair< kwto::track_handle_type, kwto::track_handle_type > track2track_type;
typedef unsigned long long ts_type;
typedef std::pair<ts_type,ts_type> ts_frame_range;

// state of the frame-has-been-matched flag: outside aoi, in_aoi_unmatched, in_aoi_matched
// order so that IN_AOI_UNMATCHED is zero (default).
enum FRAME_MATCH_STATE { IN_AOI_UNMATCHED = 0, OUTSIDE_AOI, IN_AOI_MATCHED };

struct SCORE_CORE_EXPORT scorable_track_type: public kwto::track_base< scorable_track_type >
{
  kwto::track_field< kwto::dt::tracking::external_id > external_id;
  kwto::track_field< kwto::dt::tracking::bounding_box > bounding_box;
  kwto::track_field< kwto::dt::tracking::frame_number > timestamp_frame;
  kwto::track_field< kwto::dt::tracking::timestamp_usecs > timestamp_usecs;
  kwto::track_field< int >& frame_has_been_matched;
  kwto::track_field< unsigned >& frames_in_aoi;
  scorable_track_type()
    : frame_has_been_matched( Frame.add_field< int >( "frame_has_been_matched" )),
      frames_in_aoi(Track.add_field< unsigned >("frames_in_aoi"))
  {
    Track.add_field( external_id );
    Frame.add_field( bounding_box );
    Frame.add_field( timestamp_frame );
    Frame.add_field( timestamp_usecs );
  }
};

} // ...kwiver
} // ...kwant


#endif

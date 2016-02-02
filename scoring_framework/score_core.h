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
#include <track_oracle/track_oracle.h>
#include <track_oracle/track_base.h>
#include <track_oracle/track_field.h>

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

typedef std::pair< track_handle_type, track_handle_type > track2track_type;
typedef unsigned long long ts_type;
typedef std::pair<ts_type,ts_type> ts_frame_range;

// state of the frame-has-been-matched flag: outside aoi, in_aoi_unmatched, in_aoi_matched
// order so that IN_AOI_UNMATCHED is zero (default).
enum FRAME_MATCH_STATE { IN_AOI_UNMATCHED = 0, OUTSIDE_AOI, IN_AOI_MATCHED };

struct SCORE_CORE_EXPORT scorable_track_type: public track_base< scorable_track_type >
{
  track_field< unsigned >& external_id;
  track_field< vgl_box_2d<double> >& bounding_box;
  track_field< unsigned >& timestamp_frame;
  track_field< ts_type >& timestamp_usecs;
  track_field< int >& frame_has_been_matched;
  track_field< unsigned >& frames_in_aoi;
  scorable_track_type()
    : external_id( Track.add_field< unsigned >( "external_id" ) ),
      bounding_box( Frame.add_field< vgl_box_2d<double> >( "bounding_box" )),
      timestamp_frame( Frame.add_field< unsigned >( "frame_number" )),
      timestamp_usecs( Frame.add_field< ts_type > ( "timestamp_usecs" )),
      frame_has_been_matched( Frame.add_field< int >( "frame_has_been_matched" )),
      frames_in_aoi(Track.add_field< unsigned >("frames_in_aoi"))
  {
  }
};

} // ...kwiver
} // ...kwant


#endif

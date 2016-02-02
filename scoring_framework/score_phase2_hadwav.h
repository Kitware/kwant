/*ckwg +5
 * Copyright 2010-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_SCORE_PHASE2_HADWAV_H
#define INCL_SCORE_PHASE2_HADWAV_H

// Scoring, phase 2: this is the phase which reduces the matrix of track-to-track
// metrics to the raw ingredients for phase 3, the actual metrics phase.
// This is the phase in which, for example, assignments are optimized based on
// the phase 1 results.

#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <scoring_framework/score_core.h>
#include <scoring_framework/score_tracks_hadwav_export.h>

namespace kwiver {
namespace kwant {

struct track2track_phase1;

struct SCORE_TRACKS_HADWAV_EXPORT track2track_scalars_hadwav
{
public:
  // true if the target and computed track-match
  bool computed_associated_with_target;
  unsigned int computed_frames_on_target;

  // true if the target dominates the computed
  // ... Any computed track is dominated by zero or one target
  // ... any target track can dominate multiple computed tracks
  bool computed_is_dominated_by_target;

  // true if the computed dominates the target
  // ... Any target is dominated by zero or one computed track
  // ... any computed track can dominate multiple targets
  bool target_is_dominated_by_computed;

  track2track_scalars_hadwav()
    : computed_associated_with_target( false ),
      computed_frames_on_target( 0 ),
      computed_is_dominated_by_target( false ),
      target_is_dominated_by_computed( false )
  {}
};

struct SCORE_TRACKS_HADWAV_EXPORT track2track_phase2_hadwav
{
public:
  std::map< track2track_type, track2track_scalars_hadwav> t2t;
  std::map< track_handle_type, track_handle_list_type > c2t;  // key = computed ID; val = list of associated truth tracks
  std::map< track_handle_type, track_handle_list_type > t2c;  // key = truth track ID; val = list of associated computed tracks
  size_t n_true_tracks;
  size_t n_computed_tracks;

  double framePD;
  double frameFA;
  double trackFramePrecision;
  double detectionPD;
  size_t detectionFalseAlarms;
  double detectionPFalseAlarm;
  bool verbose;

  void compute( const track_handle_list_type& t, const track_handle_list_type& c, const track2track_phase1& p1 );
  void debug_dump( std::ostream& os );

  explicit track2track_phase2_hadwav( bool v = false ) :
    n_true_tracks(0),
    n_computed_tracks(0),
    framePD(0.0),
    frameFA(0.0),
    trackFramePrecision(0.0),
    detectionPD(0.0),
    detectionFalseAlarms(0),
    detectionPFalseAlarm(0),
    verbose(v)
  {}
};

} // ...kwant
} // ...kwiver

#endif

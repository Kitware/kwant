/*ckwg +5
 * Copyright 2010 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_SCORE_PHASE2_AIPR_H
#define INCL_SCORE_PHASE2_AIPR_H

// Scoring, phase 2: this is the phase which reduces the matrix of track-to-track
// metrics to the raw ingredients for phase 3, the actual metrics phase.
// This is the phase in which, for example, assignments are optimized based on
// the phase 1 results.

#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <track_oracle/scoring_framework/score_core.h>

namespace vidtk
{

struct track2track_phase1;

struct track2track_scalars_aipr
{
public:
  bool computed_associated_with_target;
  ts_frame_range associtaion_range;
  size_t associated_frame_count;
  double association_value;
  track2track_scalars_aipr()
    : computed_associated_with_target( false ),
      associated_frame_count( 0 ),
      association_value( -1 )
  {}
};

struct track2track_phase2_aipr
{
public:
  std::map< track2track_type, track2track_scalars_aipr> t2t;
  std::map< track_handle_type, track_handle_list_type > c2t;  // key = computed ID; val = list of associated truth tracks
  std::map< track_handle_type, track_handle_list_type > t2c;  // key = truth track ID; val = list of associated computed tracks
  size_t n_true_tracks;
  size_t n_computed_tracks;

  void compute( const track_handle_list_type& t, const track_handle_list_type& c, const track2track_phase1& p1 );
};

} // namespace vidtk

#endif

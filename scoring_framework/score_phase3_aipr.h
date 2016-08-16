/*ckwg +5
 * Copyright 2010 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_SCORE_PHASE3_AIPR_H
#define INCL_SCORE_PHASE3_AIPR_H

// In phase 3, the actual metrics are produced.

#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <track_oracle/scoring_framework/score_core.h>

namespace vidtk
{

struct track2track_phase2_aipr;

struct overall_phase3_aipr
{
public:

  //number of computed tracks associated with multiple ground truths
  int num_identity_switch;
  //number of computed tracks associated with multiple ground truths divided by total number of computed tracks
  double identity_switch;

  //sum of all frame ranges of associated computed tracks over the sum of all frame ranges of ground truth tracks
  double track_completeness_factor;

  //Sum of the number of computed id's associated with a ground truth divided by the number of ground truths with associations
  double track_fragmentation;

  //sum of comp associated with a ground truth * the length of the ground truth all divided by the frame ranges of ground truths with associations
  double normalized_track_fragmentation;

  public:
    overall_phase3_aipr():
          num_identity_switch(0),
          identity_switch(0),
          track_completeness_factor(0),
          track_fragmentation(0),
          normalized_track_fragmentation(0)
    {}
    void compute( const track2track_phase2_aipr& t2t );
};

} //namespace vidtk

#endif

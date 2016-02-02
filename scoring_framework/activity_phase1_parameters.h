/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_ACTIVITY_PHASE1_PARAMETERS_H
#define INCL_ACTIVITY_PHASE1_PARAMETERS_H

#include <track_oracle/track_oracle.h>
#include <track_oracle/scoring_framework/score_core.h>
#include <vgl/vgl_box_2d.h>

namespace vidtk
{

struct activity_phase1_parameters
{
public:
  double min_percentage_of_event_matches;
  double min_number_of_event_maches;
  //The min overlap diff(max(gt_begin,cp_begin),min(gt_end,cp_end))/diff(gt_begin, gt_end)
  double min_percentage_time_overlap;
  double min_number_of_frames_overlap;


  activity_phase1_parameters()
    : min_percentage_of_event_matches(0),
      min_number_of_event_maches(1),
      min_percentage_time_overlap(0),
      min_number_of_frames_overlap( 1 )
  {}
};

} // namespace vidtk

#endif

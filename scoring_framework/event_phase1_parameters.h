/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_EVENT_PHASE1_PARAMETERS_H
#define INCL_EVENT_PHASE1_PARAMETERS_H

#include <track_oracle/core/track_oracle.h>
#include <scoring_framework/score_core.h>
#include <vgl/vgl_box_2d.h>

namespace vidtk
{

struct event_phase1_parameters
{
public:
  //The min overlap diff(max(gt_begin,cp_begin),min(gt_end,cp_end))/diff(gt_begin, gt_end)
  double min_percentage_fragment_ctrack_and_gtrack_overlap;
  double min_percentage_track_matches;
  double min_number_of_track_matches;
  //The min overlap diff(max(gt_begin,cp_begin),min(gt_end,cp_end))/diff(gt_begin, gt_end)
  double min_percentage_event_time_overlap;
  double min_number_of_track_segments_frames_overlap_matches;
  double min_number_of_event_frame_overlap;
  //For a match to be considered the best, both the gt and cp must have it has their first match
  bool do_bi_directional_matching_tp;
  bool do_bi_directional_matching_fp;


  event_phase1_parameters()
    : min_percentage_fragment_ctrack_and_gtrack_overlap(0),
      min_percentage_track_matches(0),
      min_number_of_track_matches(1),
      min_number_of_track_segments_frames_overlap_matches( 1 ),
      min_number_of_event_frame_overlap(1),
      do_bi_directional_matching_tp(true),
      do_bi_directional_matching_fp(true)
  {}
};

} // namespace vidtk

#endif

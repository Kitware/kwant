/*ckwg +5
 * Copyright 2010 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef score_frames_aipr_h_
#define score_frames_aipr_h_

#include <vector>
#include <tracking_data/track.h>

struct frame_based_metrics
{
  double true_positives_count;
  double false_negatives_count;
  double false_positives_count;

  double true_positives_presence;

  double detection_probability_count;
  double detection_probability_presence;

  double precision_count;
  double precision_presence;

  double num_split;
  //If more than 2 tracks occur in the split
  double num_split_mult;
  double split_fraction;

  double num_merge;
  //If more than 2 tracks occur in the merge
  double num_merge_mult;
  double merge_fraction;

  double gt_with_associations;
  double gt_without_associations;
};


void compute_metrics(std::vector<frame_based_metrics>& fbm,
                     std::vector<vidtk::track_sptr>& gt_tracks,
                     std::vector<vidtk::track_sptr>& comp_tracks,
                     double min_overlap_ratio = 0);

#endif

/*ckwg +5
 * Copyright 2010-2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_MATCHING_ARGS_TYPE_H
#define INCL_MATCHING_ARGS_TYPE_H

///
/// This type defines the various options passed to the phase1 object
/// to control what it means for two detections to overlap.
///

#include <vital/vital_config.h>
#include <scoring_framework/score_core_export.h>

#include <string>
#include <utility>
#include <vul/vul_arg.h>

namespace kwiver {
namespace kwant {

struct SCORE_CORE_EXPORT matching_args_type
{
  vul_arg<double> frame_alignment_secs;
  vul_arg<double> bbox_expansion;
  vul_arg<std::string> aoi_string;
  vul_arg<double> min_bound_area;
  vul_arg<std::string> min_frames_arg;
  vul_arg<std::string> min_pcent_gt_ct;
  vul_arg<double> iou;
  vul_arg<double> radial_overlap;
  vul_arg<bool> pass_nonzero_overlaps;
  std::pair< bool, double > min_frames_policy;  // first: true if absolute, false if percentage
  matching_args_type()
  : frame_alignment_secs( "--frame-align", "timestamp difference (secs) between aligned frames in truth and test data", 1.0 / 2.0 ),
    bbox_expansion( "--expand-bbox", "expand bounding boxes in meters (if gsd given) or pixels (if not)" ),
    aoi_string( "--aoi", "Scoring AOI: either pixel, 2-corner lon/lat, or 4-corner lon/lat; 'help' for more details" ),
    min_bound_area( "--match-overlap-lower-bound", "frame overlap must be > this to match", 0.0 ),
    min_frames_arg( "--match-frames-lower-bound", "if != 0, at least this many frames must overlap for track to match; 'p' for %age of gt", "0" ),
    min_pcent_gt_ct( "--min-pcent-gt-ct", "%age of gt/ct box which must be overlapped; set to 'help' for more info; overrides match-overlap-lower-bound" ),
    iou( "--iou", "intersection-over-union: ratio of overlap to union of bounding boxes; overrides match-overlap-lower-bound; test is >=" ),
    radial_overlap( "--radial-overlap", "-1.0 to disable; otherwise, distance in meters between detections to match", -1.0 ),
    pass_nonzero_overlaps( "--pass-nonzero-overlaps", "if set, ALL frames with nonzero overlaps will be used for stats" ),
    min_frames_policy( std::make_pair( false, 0.0 ))
  {}

  bool sanity_check() const;
  bool parse_min_pcent_gt_ct( std::pair<double, double>& p, bool& debug_flag ) const;
  bool parse_min_frames_arg();
};

} // ...kwant
} // ...kwiver

#endif

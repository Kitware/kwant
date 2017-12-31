/*ckwg +5
 * Copyright 2010-2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_PHASE1_PARAMETERS_H
#define INCL_PHASE1_PARAMETERS_H

#include <vital/vital_config.h>
#include <scoring_framework/score_core_export.h>

#include <vgl/vgl_box_2d.h>
#include <vgl/vgl_polygon.h>

#include <track_oracle/core/track_oracle_core.h>
#include <scoring_framework/score_core.h>
#ifdef KWANT_ENABLE_MGRS
#include <track_oracle/file_formats/track_scorable_mgrs/scorable_mgrs.h>
#endif

namespace kwiver {
namespace kwant {

namespace kwto = ::kwiver::track_oracle;

struct matching_args_type;

struct SCORE_CORE_EXPORT phase1_frame_window
{
  ts_type f0, f1;
  bool is_set;
  phase1_frame_window(): f0(0), f1(0), is_set( false ) {}
  phase1_frame_window( ts_type F0, ts_type F1 ): f0(F0), f1(F1), is_set( true ) {}
};

struct SCORE_CORE_EXPORT mgrs_aoi
{
  vgl_polygon<double> aoi;
  int zone;
  mgrs_aoi(): zone(-1) {}
  mgrs_aoi( const vgl_polygon<double>& a, int z ): aoi(a), zone(z) {}
};

struct SCORE_CORE_EXPORT phase1_parameters
{
  bool expand_bbox;

  double bbox_expansion;

  double frame_alignment_time_window_usecs;

  // (Jon: What does this mean?  I'm assuming it means: if a point p is in the
  // AOI and aoiInclusive is true, then the point counts; if a point is outside
  // the AOI and aoiInclusive is false, then the point counts. -- Roddy)
  bool aoiInclusive; //Is AOI inclusive or exclusive?

  // AOI processing: AOIs may be specified in either pixels (b_aoi)
  // or lat/lon (mgrs_aoi), but not both.  mgrs_aoi_list is only checked when
  // radial_overlap >= 0.  If the mgrs_aoi_list is empty, then no AOI
  // was requested.
  //
  // If no (pixel) AOI is set then the entire frame is an AOI.
  //
  vgl_box_2d<double> b_aoi;
  std::vector< mgrs_aoi > mgrs_aoi_list;

  // if set, results will only be computed in this frame window
  phase1_frame_window frame_window;

  bool perform_sanity_checks;

  // on a single frame, area must be > min_bound_matching_area to match
  double min_bound_matching_area;

  // (f,s) = min_frames_policy.(first,second)
  // if (f == true) (absolute): entire track must have > s frames to match
  // if (f == false) (percentage): number of matches must be > s% of ground truth length
  std::pair< bool, double > min_frames_policy;

 // percentage of (gt, ct) boxes which must be overlapped for it to be a hit; <0 for don't care
  std::pair<double, double> min_pcent_overlap_gt_ct;

  // intersection-over-union; not compatible with min_pcent_overlap_gt_ct
  double iou;

  // if true, dump statistics
  bool debug_min_pcent_overlap_gt_ct;

  // radial overlap: -1 (or anything <0) to disable and use bounding boxes.  Otherwise,
  // distance in meters between two detections for them to match.
  double radial_overlap;

  // when using radial overlap, to test for inclusion in an AOI, convert point-only
  // detections into a square box this many pixels on a side, centered on the detection.
  // Not used when radial_overlap < 0.
  unsigned point_detection_box_size;

  // if true, all non-zero overlaps will passed downstream for
  // computing e.g. purity and continuity.  If false, only those
  // frames passing the frame filters (min_bound_matching_area,
  // min_matching_frames, min_pcent_overlap_gt_ct) are passed
  // downstream.  Invalid if radial overlap requested.
  bool pass_all_nonzero_overlaps;


  phase1_parameters()
    : expand_bbox(false),
      bbox_expansion(0.0),
      frame_alignment_time_window_usecs( 1.0 / 30.0 * 1.0e6 ),
      aoiInclusive(false),
      perform_sanity_checks(true),
      min_bound_matching_area(0.0),
      min_frames_policy( std::make_pair( true, 0.0 )),
      min_pcent_overlap_gt_ct( std::make_pair(-1.0, -1.0) ),
      iou ( -1.0 ),
      debug_min_pcent_overlap_gt_ct( false ),
      radial_overlap( -1.0 ),
      pass_all_nonzero_overlaps( false )
  {}

  explicit phase1_parameters(double expansion)
    : expand_bbox(true),
      bbox_expansion(expansion),
      frame_alignment_time_window_usecs( 1.0 / 30.0 * 1.0e6 ),
      aoiInclusive(false),
      perform_sanity_checks(true),
      min_bound_matching_area(0.0),
      min_frames_policy( std::make_pair( true, 0.0 )),
      min_pcent_overlap_gt_ct( std::make_pair(-1.0, -1.0) ),
      iou( -1.0 ),
      debug_min_pcent_overlap_gt_ct( false ),
      radial_overlap( -1.0 ),
      pass_all_nonzero_overlaps( false )
  {}

  bool processMatchingArgs( const matching_args_type& m );

  void setAOI(vgl_box_2d<double> bbox, bool inclusive );
  bool setAOI( const std::string& aoi_string );

  void set_frame_window( ts_type f0, ts_type f1 );

  // remove tracks which, even after bounding box expansion, do not
  // have an "AOI match" (in the AOI if inclusive, outside if exclusive)
  // If frame window is set, also tosses tracks which are entirely outside
  // the frame window.
  //
  // Return the min and max timestamps in the filtered track list, for
  // later normalization.

  std::pair<ts_type, ts_type> filter_track_list_on_aoi( const kwto::track_handle_list_type& in,
                                                        kwto::track_handle_list_type& out );

private:
  enum AOI_STATUS {NO_AOI_USED, PIXEL_AOI, GEO_AOI};

  AOI_STATUS get_aoi_status() const;
  bool frame_within_pixel_aoi( const kwto::frame_handle_type& f ) const;
  bool frame_within_geo_aoi( const kwto::frame_handle_type& f ) const;

};

} // ...kwant
} // ...kwiver

#endif

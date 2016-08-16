/*ckwg +5
 * Copyright 2010-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_SCORE_PHASE1_H
#define INCL_SCORE_PHASE1_H

#include <vital/vital_config.h>
#include <scoring_framework/score_core_export.h>

#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <limits>
#include <scoring_framework/score_core.h>
#include <scoring_framework/phase1_parameters.h>
#include <track_oracle/vibrant_descriptors/descriptor_overlap_type.h>
#include <track_oracle/vibrant_descriptors/descriptor_event_label_type.h>

//
// In phase 1, each ground truth track is compared to each computed track
// independently of all other possible pairings and reduced to a matrix
// of track-to-track score objects.  Ideally, all further scoring can
// be based off these objects (plus the optional phase contexts) without
// needing to ever look again at the tracks themselves.  If you need to
// look at the tracks, then whatever it was you were looking for should
// probably go in this structure instead.


namespace kwiver {
namespace kwant {

namespace kwto = ::kwiver::track_oracle;

//
// the track2track_score contains the results of compairing a single
// pair of tracks.
//

kwto::frame_handle_list_type
sort_frames_by_field( kwto::track_handle_type track_id, const std::string& name);


struct SCORE_CORE_EXPORT track2track_frame_overlap_record
{
public:
  kwto::frame_handle_type truth_frame;
  kwto::frame_handle_type computed_frame;
  unsigned int fL_frame_num; // frame number of first overlapping frame
  unsigned int fR_frame_num; // frame number of second overlapping frame
  double truth_area;    // area of ground-truth bounding box
  double computed_area; // area of computed bounding box
  double overlap_area;  // area of overlap between the two
  double centroid_distance;  // distance between centroid of boxes
  double center_bottom_distance; // distance between "feet" of boxes
  bool in_aoi; // if the result is computed on two boxes with an AOI match (see below)
  track2track_frame_overlap_record()
    : fL_frame_num(0),
      fR_frame_num(0),
      truth_area(0.0),
      computed_area(0.0),
      overlap_area(0.0),
      centroid_distance(0.0),
      center_bottom_distance(0.0),
      in_aoi(true)
  {
  }
};

struct SCORE_CORE_EXPORT track2track_score
{
public:
  unsigned spatial_overlap_total_frames;
  ts_frame_range overlap_frame_range;
  std::vector< track2track_frame_overlap_record > frame_overlaps;
  track2track_score()
    : spatial_overlap_total_frames(0),
      overlap_frame_range(std::numeric_limits<ts_type>::max(),std::numeric_limits<ts_type>::max())
  {
  }

  // fill in the values given truth track t and computed track c
  // returns false if tracks are not in the AOI
  bool compute( kwto::track_handle_type t,
                kwto::track_handle_type c,
                const phase1_parameters& params );

  // line up the two frame lists with a tolerance of match_window
  // and return a list of aligned frame handles
  std::vector< std::pair< kwto::frame_handle_type, kwto::frame_handle_type > >
  align_frames( const kwto::frame_handle_list_type& f1,
                const kwto::frame_handle_list_type& f2,
                double match_window );

  // given two frames, return their spatial overlap
  track2track_frame_overlap_record compute_spatial_overlap( kwto::frame_handle_type f1,
                                                            kwto::frame_handle_type f2,
                                                            const phase1_parameters& params );

#ifdef KWANT_ENABLE_MGRS
  // given two frames, return their radial overlap (throw if param not set)
  track2track_frame_overlap_record compute_radial_overlap( kwto::frame_handle_type f1,
                                                           kwto::frame_handle_type f2,
                                                           const phase1_parameters& params );

#endif

  bool within_window( const kwto::frame_handle_list_type& f1,
                      const kwto::frame_handle_list_type& f2,
                      unsigned f1_ptr,
                      unsigned f2_ptr,
                      double match_window );

  kwto::descriptor_overlap_type create_overlap_descriptor() const;
  void add_self_to_event_label_descriptor( kwto::descriptor_event_label_type& delt ) const;

private:
  // cached for the descriptor
  kwto::track_handle_type cached_truth_track, cached_comp_track;

  bool move_a_up_to_b( unsigned& index,
                       const kwto::frame_handle_list_type& lagging_list,
                       unsigned fixed_index,
                       const kwto::frame_handle_list_type& fixed_list,
                       double match_window );
};

struct SCORE_CORE_EXPORT track2track_phase1
{
public:
  // key: (gt_handle, computed_handle)   value: resulting t2t_score
  std::map< track2track_type, track2track_score > t2t;

  track2track_phase1()
  {}
  explicit track2track_phase1( const phase1_parameters& new_params):
        params(new_params)
  {}

  void compute_all( const kwto::track_handle_list_type& t,
                    const kwto::track_handle_list_type& c );

  bool compute_single( kwto::track_handle_type t, kwto::track_handle_type c);

  void debug_dump( const kwto::track_handle_list_type& gt_list,
                   const kwto::track_handle_list_type& ct_list,
                   const std::string& fn_prefix,
                   ts_type ts_offset ) const;

  void debug_dump( std::ostream& os );

  std::vector< track2track_type > matching_keys( const track2track_type& probe ) const;

  ts_type min_ts() const;

  phase1_parameters params;
};

} // ...kwant
} // ...kwiver

#endif

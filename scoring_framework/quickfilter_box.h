/*ckwg +5
 * Copyright 2012-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_QUICKFILTER_BOX_H
#define INCL_QUICKFILTER_BOX_H

#include <vital/vital_config.h>
#include <scoring_framework/score_core_export.h>

#include <vgl/vgl_box_2d.h>
#include <vgl/vgl_intersection.h>

#include <track_oracle/core/track_base.h>
#ifdef KWANT_ENABLE_MGRS
#include <track_oracle/file_formats/track_scorable_mgrs/scorable_mgrs.h>
#endif

namespace kwiver {
namespace kwant {

namespace kwto = ::kwiver::track_oracle;

struct phase1_parameters;

//
// This structure enables quick-filtering on spatial extent
// by holding a bounding box containing the entire track; for
// any two tracks, if these boxes do not intersect, then we can
// skip a temporally-ordered frame-by-frame comparison.
//
// The coordinate system of the track-level bounding box is determined
// by the overlap criterion: spatial overlap is in image coordinates;
// radial overlap is in MGRS.  (Tracks which cross MGRS boundaries
// will have no quick-filter box and thus will always pass the quick-
// filter test.)
//
// One instance of this schema is associated with a track; the
// appropriate add_ method is called for each frame; at the end,
// you have the bounding box of the track in the appropriate coord
// system. (The coord system is not set explicitly, but instead
// set the first time an add_ method is called.)
//
// The implementation is an awkward example of a track oracle union.
//

struct SCORE_CORE_EXPORT quickfilter_box_type: public kwto::track_base< quickfilter_box_type >
{
  enum {COORD_NONE=0, COORD_IMG, COORD_MGRS};
  // what coordinate system is the bounding box?
  kwto::track_field< int >& coord_system;

  // valid only if coord_system is COORD_IMG.
  kwto::track_field< vgl_box_2d<double> >& img_box;

#ifdef KWANT_ENABLE_MGRS
  // valid only if coord_system is COORD_MGRS.
  // It's tempting to have a vgl_box_2d<double> of northing/easting
  // to avoid having to duplicate all vgl_box_2d's nice logic for
  // adding to a bounding box, but on the other hand scorable_mgrs
  // already handles arbitrating between MGRS zones.
  kwto::track_field< kwto::scorable_mgrs >& sw_point;
  kwto::track_field< kwto::scorable_mgrs >& ne_point;
#endif

  // relevant only if coord_system is COORD_MGRS.
  // True if all points added so far have at least one
  // compatible zone.  Initialized to true; once false, stays
  // false.
  kwto::track_field< bool >& mgrs_valid_latch;

  quickfilter_box_type():
    coord_system( Track.add_field< int >( "qf_box_coord_system" )),
    img_box( Track.add_field< vgl_box_2d<double> >( "qf_box_img_box" )),
#ifdef KWANT_ENABLE_MGRS
    sw_point( Track.add_field< kwto::scorable_mgrs >( "qf_box_sw_point" )),
    ne_point( Track.add_field< kwto::scorable_mgrs >( "qf_box_ne_point" )),
#endif
    mgrs_valid_latch( Track.add_field<bool>( "mgrs_valid_latch" ))
  {}

  // these are initialized to invalid; set to (truth, computed)
  // external IDs to trigger debugging
  static std::pair< unsigned, unsigned > debug_track_ids;

  // client's main function to add instances of this data
  // structure to the track list
  static void add_quickfilter_boxes( const kwto::track_handle_list_type& t,
                                     const phase1_parameters& params );

#ifdef KWANT_ENABLE_MGRS
  // incorporates s such that {sw,ne}_point is the convex hull of the points
  // and s.  Throws if coord_system == coord_img.
  void add_scorable_mgrs( const kwto::track_handle_type& t,
                          kwto::scorable_mgrs s );
#endif

  // Adds box to the box already associated with the track.
  // Throws if coord_system == coord_mgrs.
  void add_image_box( const kwto::track_handle_type& t,
                      vgl_box_2d<double> box );

#ifdef KWANT_ENABLE_MGRS
  // If either (or both) of t1/t2 have coord_system != coord_mgrs, return -1.
  // If either (or both) of t1/t2 contain invalid {se,ne} points, return -1.
  // If t1/t2 do not have compatible zones for both sets of {sw,ne}_points, return 0.
  // Otherwise, t1/t2 have compatible MGRS boxes; return area of overlap (or 0 if none.)
  // (The idea is that -1 means "you need to examine t1 and t2 on a frame-by-frame
  // basis", while >=0 is a valid result.)
  double mgrs_box_intersect( const kwto::track_handle_type& t1,
                             const kwto::track_handle_type& t2 );

#endif
  double img_box_intersect( const kwto::track_handle_type& t1,
                            const kwto::track_handle_type& t2 );

  // return >=0 if valid boxes could be compared; return -1 if no
  // quickfilter decision could be made.
  double quickfilter_check( const kwto::track_handle_type& t1,
                            const kwto::track_handle_type& t2,
                            bool use_radial_overlap );

};

} // ...kwant
} // ...kwiver

#endif

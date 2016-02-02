/*ckwg +5
 * Copyright 2013-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_SYNTHESIZER_H
#define INCL_TRACK_SYNTHESIZER_H

///
/// This factory generates simple tracks for the purposes of testing
/// track scoring code.  The tracks are instances of scorable_track_type.
///
/// The user passes a string to the factory which describes how up to
/// three tracks (ids 1, 2, and 3) can intersect.  Each character in
/// the string represents a frame and may be one of:
///
/// '.' - no frames at all
/// 'a', 'b', 'c' - a single box from one of tracks {1, 2, 3}
/// 'd', 'e', 'f' - non-overlapping boxes from tracks { (1,2), (2,3), (1,3) }
/// 'g',          - three non-overlapping boxes from all tracks (1,2,3)
/// 'x' - tracks 1 & 2 intersect, 3 not present
/// 'X' - tracks 1 & 2 intersect, 3 present but does not intersect
/// 'y' - tracks 1 & 3 intersect, 2 not present
/// 'Y' - tracks 1 & 3 intersect, 2 present but does not intersect
/// 'z' - tracks 2 & 3 intersect, 1 not present
/// 'Z' - tracks 2 & 3 intersect, 1 present but does not intersect
/// '#' - tracks 1, 2, and 3 all intersect
///
/// Note that having 1 & 2 intersect, 2 & 3 intersect,
/// but 1 & 3 *not* intersect is not supported.  (See comments
/// in box_pool below for the reason why.)
///
///
/// *************
/// *************
/// NOTE!
/// *************
/// *************
///
/// 'a', 'b', 'c' are TRACK IDs, NOT spatial locations.
/// See track_synthesizer.cxx for an explanation why if
/// you have a track string "aaaa", and another track "ccaa",
/// there is 100% spatial overlap between the two strings.
/// (Basically the three pre-set spatial locations are
/// allocated on a first-come, first-serve per-frame basis.)
///
/// Some examples:
///
/// "....aaa...bbb.."
/// : tracks 1 and 2 are each three frames long and do not intersect
///
/// "....aaxbb.."
/// : tracks 1 and 2 are each three frames long and intersect on frame 6
///
/// "..a.b.c.a.b.c.X.Z.##.a.b.c.."
/// : tracks 1, 2, and 3 move along independently for a while, start
/// to merge, overlap for two frames, then move apart again
///

#include <scoring_framework/score_core.h>

namespace kwiver {
namespace kwant {

//
// spatio-temporal parameters for the detections
//
// overlapping is achieved by sliding box #2 down
// relative to box #1.  When three boxes overlap, the
// third box is positioned so that all the centroids
// are positioned in an equilateral triangle.  The
// idea is that the overlap_distance also functions as
// the radial overlap distance parameter for non-spatial
// overlap detection.  The drawback is that this makes it
// trickier to precisely predict the amount of spatial
// overlap.
//
struct track_synthesizer_params
{
  double box_side_length; // boxes are square
  double overlap_distance; // must be in range [0, box_side_length)
  double fps;  // for computing timestamps
  track_synthesizer_params( double b, double o, double f);
};


class box_pool
{
public:
  explicit box_pool( const track_synthesizer_params& params );
  vgl_box_2d<double> get_box( char box_code ) const;

private:
  std::map< char, vgl_box_2d<double> > boxes;
};


// each call to make_tracks generates independent sets of
// tracks (track IDs restart at zero, etc.)

class track_synthesizer
{
public:
  explicit track_synthesizer( const track_synthesizer_params& p )
  : params( p ), bp( p )
  {}

  bool make_tracks( const std::string& track_description_string,
                    track_handle_list_type& output_tracks );

private:
  track_synthesizer_params params;
  box_pool bp;
  track_handle_list_type tracks;

  bool create_boxes( char c, unsigned frame_number, ts_type ts );
  void add_box_to_track( unsigned id, unsigned frame_number, ts_type ts, const vgl_box_2d<double>& box );

};

} // ...kwant
} // ...kwiver

#endif

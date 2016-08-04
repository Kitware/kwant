/*ckwg +5
 * Copyright 2013-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "track_synthesizer.h"

#include <vnl/vnl_math.h>
#include <stdexcept>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::map;
using std::ostringstream;
using std::runtime_error;
using std::sqrt;
using std::string;

using kwiver::track_oracle::frame_handle_type;
using kwiver::track_oracle::track_handle_list_type;
using kwiver::track_oracle::track_handle_type;

namespace kwiver {
namespace kwant {

track_synthesizer_params
::track_synthesizer_params( double b, double o, double f)
 : box_side_length(b), overlap_distance(o), fps(f)
{
  if (overlap_distance >= box_side_length)
  {
    ostringstream oss;
    oss << "track_synthesizer_params: overlap distance " << overlap_distance
        << " must be < box side length " << box_side_length << "\n";
    throw runtime_error( oss.str().c_str() );
  }
};

//
// There are only six distinct box topologies, once you factor out
// the track ID assignment:
//
// (1) '.'            - no boxes
// (2) 'a', 'b', 'c'  - one box
// (3) 'd', 'e',      - two non-overlapping boxes
// (4) 'f'            - three non-overlapping boxes
// (5) 'x', 'y', 'z'  - two overlapping boxes
// (6) 'X', 'Y', 'Z'  - two overlapping and one non-overlapping
// (7) '#'            - three overlapping boxes
//
//
// Once the track_synthesizer_params are fixed, we can precompute the
// layout of all the boxes, which are then stored in the box_pool and
// mapped to track IDs based on the box descriptor code.  The box_pool
// tracks are labeled:
//
// I, J, K:  three non-overlapping boxes
// L, M:     two (mutually) overlapping boxes which overlap I such that
//           all their centroids are overlap_distance apart.
//
// (I,L,M) describe an equilateral triangle; J and K sit off to the
// side.  The relative positioning of I,L, and M is the reason why
// having tracks 1&2 intersect, and 1&3 intersect, but 1&3 be disjoint
// is not supported.
//
// For example, the descriptor characters map to track IDs thusly:
//
// 'c': I->3  (could be J or K as well)
// 'e': I->2, J->3  (again, there are other possibilities)
// 'X': I->1, L->2
// '#': I->1, L->2, M->3
//

box_pool
::box_pool( const track_synthesizer_params& params )
{
  double s = params.box_side_length;
  double no_overlap_offset = 10*s;
  double offset = 0.0;
  // three non-overlapping boxes
  this->boxes[ 'I' ] = vgl_box_2d<double>( offset, offset+s, 0.0, s );
  offset += no_overlap_offset;
  this->boxes[ 'J' ] = vgl_box_2d<double>( offset, offset+s, 0.0, s );
  offset += no_overlap_offset;
  this->boxes[ 'K' ] = vgl_box_2d<double>( offset, offset+s, 0.0, s );

  // two boxes overlapping with 'I' such that the centroid
  // distances are all ov
  double ov = params.overlap_distance;
  this->boxes[ 'L' ] = vgl_box_2d<double>( 0, s, ov, ov+s );

  // a little trig to position 'M'
  double x_offset = -1.0 * sqrt( 3.0/4.0*(ov*ov) ); // -1.0 just to move it off to the left
  double y_offset = ov / 2.0;
  this->boxes[ 'M' ] = vgl_box_2d<double>( x_offset, x_offset+s, y_offset,  y_offset+s );
}

vgl_box_2d<double>
box_pool
::get_box( char box_code ) const
{
  map< char, vgl_box_2d<double> >::const_iterator i = boxes.find( box_code );
  if (i == boxes.end() )
  {
    ostringstream oss;
    oss << "Bad box character '" << box_code << "'";
    throw runtime_error( oss.str().c_str() );
  }
  return i->second;
}

//
// Given a track description character, add
// the box(es) to the frame on the track(s).
//

bool
track_synthesizer
::create_boxes( char c,
                unsigned frame_number,
                ts_type ts )
{
  // a list of up to three XD pairs, where
  // X = {I,J,K,L,M}
  // D = {1,2,3}
  string mapping;
  switch (c)
  {
  case '.':  mapping = "";  break;
  case 'a':  mapping = "I1"; break;
  case 'b':  mapping = "I2"; break;
  case 'c':  mapping = "I3"; break;
  case 'd':  mapping = "I1J2"; break;
  case 'e':  mapping = "I2J3"; break;
  case 'f':  mapping = "I1J3"; break;
  case 'g':  mapping = "I1J2K3"; break;
  case 'x':  mapping = "I1L2"; break;
  case 'X':  mapping = "I1L2J3"; break;
  case 'y':  mapping = "I1L3"; break;
  case 'Y':  mapping = "I1L3J2"; break;
  case 'z':  mapping = "I2L3"; break;
  case 'Z':  mapping = "I2L3J1"; break;
  case '#':  mapping = "I1L2M3"; break;
  default:
    LOG_ERROR( main_logger, "bad box description character '" << c << "'" );
    return false;
  }

  // move down the string picking off pairings of box codes to track IDs
  size_t cursor = 0;
  while (cursor < mapping.size())
  {
    unsigned box_code = mapping[cursor++];
    unsigned track_id = mapping[cursor++] - '0';
    this->add_box_to_track( track_id, frame_number, ts, this->bp.get_box( box_code ) );
  }

  return true;
}

//
// Add a single box to the track.
// If the track does not exst in our track list, add it.
//

void
track_synthesizer
::add_box_to_track( unsigned id,
                    unsigned frame_number,
                    ts_type ts,
                    const vgl_box_2d<double>& box )
{
  scorable_track_type t;

  // where is the track in our track list?
  track_handle_type this_track;
  for (size_t i=0; (! this_track.is_valid()) && (i<this->tracks.size()); ++i)
  {
    if ( t( this->tracks[i] ).external_id() == id ) this_track = this->tracks[i];
  }
  if ( ! this_track.is_valid() )
  {
    this_track = t.create();
    t( this_track ).external_id() = id;
    this->tracks.push_back( this_track );
  }

  // create a frame
  frame_handle_type f = t( this_track ).create_frame();
  t[ f ].bounding_box() = box;
  t[ f ].timestamp_frame() = frame_number;
  t[ f ].timestamp_usecs() = ts;
}

bool
track_synthesizer
::make_tracks( const string& track_descriptor_string,
               track_handle_list_type& output_tracks )
{
  output_tracks.clear();
  this->tracks.clear();
  ts_type clock_tick_usecs = static_cast<ts_type>( 1.0 / this->params.fps * 1.0e6 );
  ts_type ts = 0;
  for (unsigned frame_number = 0; frame_number < track_descriptor_string.size(); ++frame_number )
  {
    if (! this->create_boxes( track_descriptor_string[ frame_number ],
                              frame_number,
                              ts ))
    {
      return false;
    }
    ts += clock_tick_usecs;
  }
  output_tracks = this->tracks;
  return true;
}

} // ...kwant
} // ...kwiver

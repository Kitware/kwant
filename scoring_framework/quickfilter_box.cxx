/*ckwg +5
 * Copyright 2012-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "quickfilter_box.h"

#include <stdexcept>
#include <algorithm>

#include <vgl_area.h>

#ifdef KWANT_ENABLE_MGRS
#include <track_oracle/file_formats/track_scorable_mgrs/track_scorable_mgrs.h>
#endif
#include <scoring_framework/score_core.h>
#include <scoring_framework/phase1_parameters.h>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::make_pair;
using std::map;
using std::max;
using std::min;
using std::pair;
using std::runtime_error;
using std::vector;

using kwiver::track_oracle::field_handle_type;
using kwiver::track_oracle::track_handle_type;
using kwiver::track_oracle::track_handle_list_type;
using kwiver::track_oracle::frame_handle_type;
using kwiver::track_oracle::frame_handle_list_type;
using kwiver::track_oracle::track_oracle_core;

pair< unsigned, unsigned > kwiver::kwant::quickfilter_box_type::debug_track_ids = make_pair( static_cast<unsigned>(-1), static_cast<unsigned>(-1));


#undef QF_DEBUG

namespace /* anon */
{
using namespace ::kwiver::kwant;

#ifdef KWANT_ENABLE_MGRS

bool
check_debug( const track_handle_type& t,
             const track_handle_type& c )
{
  static scorable_track_type local_track_view;
  bool b1 = local_track_view(t).external_id() == quickfilter_box_type::debug_track_ids.first;
  bool b2 = local_track_view(c).external_id() == quickfilter_box_type::debug_track_ids.second;
  return (b1 && b2);
}

bool
check_debug( const track_handle_type& t )
{
  static scorable_track_type local_track_view;
  bool b1 = local_track_view(t).external_id() == quickfilter_box_type::debug_track_ids.first;
  bool b2 = local_track_view(t).external_id() == quickfilter_box_type::debug_track_ids.second;
  return (b1 || b2);
}


void
add_quickfilter_box_radial_frame( const track_handle_type& t,
                                  const frame_handle_type& f,
                                  double r )
{
  // When radial overlap is requested, we assume all we have is a
  // world point p (aka the frame's scorable_mgrs.)  We're going to
  // notionally define a bounding box centered on p such that if boxes
  // for two different frames do NOT intersect, we're guaranteed the
  // distance between the box centroids is > r.  (i.e. no false
  // negatives.)
  //
  // Practically, take p, add +/- r to both x and y, and add
  // those points to the quickfilter schema associated with the track.
  //
  static track_scorable_mgrs_type local_track_view;

  field_handle_type mgrs_field = local_track_view.mgrs.get_field_handle();
  if (! track_oracle_core::field_has_row( f.row, mgrs_field )) return;

  static quickfilter_box_type qf_box;

  scorable_mgrs m = local_track_view[ f ].mgrs();
  const int xf[] = { 1, 1, -1, -1 };
  const int yf[] = { 1, -1, -1, 1 };
  for (size_t i=0; i<4; ++i)
  {
    scorable_mgrs corner = m;
    for (int z=scorable_mgrs::ZONE_BEGIN; z<scorable_mgrs::N_ZONES; ++z)
    {
      // It's possible that these will land outside the strict MGRS zone
      // boundaries, but it should be fine for bounding boxes
      corner.northing[z] = m.northing[z] + (r * yf[i]);
      corner.easting[z] = m.easting[z] + (r * xf[i]);
    }
    qf_box.add_scorable_mgrs( t, corner );
 }
}
#endif

void
add_quickfilter_box_spatial_frame( const track_handle_type& t,
                                   const frame_handle_type& f,
                                   double r )
{
  // conceptually the same goal as add_qf_box_radial_frame, but much
  // simpler, since we're in image coordinates and don't have to worry
  // about zones, etc.
  static scorable_track_type local_track_view;
  field_handle_type bbox_field = local_track_view.bounding_box.get_field_handle();
  if ( ! track_oracle_core::field_has_row( f.row, bbox_field )) return;

  vgl_box_2d<double> box = local_track_view[ f ].bounding_box();
  box.expand_about_centroid( r );

  static quickfilter_box_type qf_box;
  qf_box.add_image_box( t, box );
}

void
add_quickfilter_box( const track_handle_type& t,
                     const phase1_parameters& params )
{
  if ( ! t.is_valid() ) return;

  bool use_radial_overlap = (params.radial_overlap >= 0.0);
  frame_handle_list_type f = track_oracle_core::get_frames( t );

  for (size_t i=0; i<f.size(); ++i)
  {
    if (use_radial_overlap)
    {
#ifdef KWANT_ENABLE_MGRS
      add_quickfilter_box_radial_frame( t, f[i], params.radial_overlap );
#else
      throw std::runtime_error("Use of radial overlap without MGRS support");
#endif
    }
    else
    {
      add_quickfilter_box_spatial_frame( t, f[i], (params.expand_bbox) ? params.bbox_expansion : 0.0 );
    }
  }
}

} // anon namespace

namespace kwiver {
namespace kwant {

#ifdef KWANT_ENABLE_MGRS
void
quickfilter_box_type
::add_scorable_mgrs( const track_handle_type& t,
                     scorable_mgrs s )
{
  bool dbg = check_debug( t );
  if (dbg) LOG_DEBUG( main_logger, "scorable mgrs: " << t.row  << " start" );
  this->Track.set_cursor( t.row );

  // if the coord_system is unset, assume this is the first time we're called
  // bootstrap ne / sw to s
  if (this->coord_system() == COORD_NONE)
  {
    this->coord_system() = COORD_MGRS;
    this->mgrs_valid_latch() = true;
    this->sw_point() = s;
    this->ne_point() = s;
    if (dbg) LOG_DEBUG( main_logger, "scorable mgrs: initializing " << t.row );
    return;
  }
  if (this->coord_system() != COORD_MGRS) throw runtime_error( "Logic error: MGRS called on image box?" );

  // exit if we've flipped the invalid switch
  if ( ! this->mgrs_valid_latch() )
  {
    if (dbg) LOG_DEBUG( main_logger, "scorable mgrs: " << t.row  << " exiting on latch" );
    return;
  }

  // make sure s aligns with ne / sw
  scorable_mgrs sw = this->sw_point();
  scorable_mgrs ne = this->ne_point();

  // The new point s brings two zones (one each for default and alt)
  // to the box defined by ne and sw.  Each of ne and sw also has two
  // zones.  If the mgrs_valid_latch is true, then ne and sw have at
  // least one (maybe two) zones in common.  Call them z1 and z2.
  // Note that z1 and z2 are the numeric zone IDs, and their location
  // in the default or alternate slots is irrelevant.
  //
  // Now we want to add s.  If s also contains z1 and z2, no problem.
  // If, however, it contains (say) z1 and z3, then we have to eliminate
  // z2 from ne and sw, since z2 can no longer contain all of (ne, sw, s).
  //
  // If s instead contains z3 and z4, then we have to eliminate BOTH z1
  // and z2 from ne and sw, which invalidates ne and sw, and this in turn
  // invalidates the whole quickfilter box for this track: there is no
  // zone which contains the track.  (This isn't a catastrophe, it just
  // means you'll need to check overlap on a frame-by-frame basis.)
  //
  // We start by building a map of all the zones used by (ne, sw, s)
  // and eliminating the ones which don't have support in all three.
  // Note that we can't just count up the number of times a zone
  // appears, since it may appear twice (once default, once alternate)
  // in a single scorable_mgrs.  So instead we set bitflags and only
  // accept if the flag is 0b111.

  map< int, size_t > zone_census;
  vector< scorable_mgrs* > pts;
  pts.push_back( &s );
  pts.push_back( &sw );
  pts.push_back( &ne );
  unsigned bitflag = 0x01;
  for (size_t i=0; i<pts.size(); ++i)
  {
    if (dbg) LOG_DEBUG( main_logger, "scorable mgrs: " << t.row << " @A: " << *(pts[i]) << " bitflag " << bitflag );
    for (int z=scorable_mgrs::ZONE_BEGIN; z < scorable_mgrs::N_ZONES; ++z )
    {
      if ( pts[i]->entry_valid[z] )
      {
        unsigned flag = zone_census[ pts[i]->zone[z] ];
        flag |= bitflag;
        zone_census[ pts[i]->zone[z] ] = flag;
      }
    }
    bitflag <<= 1;
  }
  if (dbg)
  {
    for (map< int, size_t >::const_iterator i=zone_census.begin(); i != zone_census.end(); ++i)
    {
      LOG_DEBUG( main_logger, "scorable mgrs: " << t.row << " zone census: zone " << i->first << " has " << i->second );
    }
  }
  for (size_t i=0; i<pts.size(); ++i)
  {
    for (int z=scorable_mgrs::ZONE_BEGIN; z < scorable_mgrs::N_ZONES; ++z )
    {
      if ( ( pts[i]->entry_valid[z] ) &&
           ( zone_census[ pts[i]->zone[z] ] != 0x07 ))
      {
        pts[i]->mark_zone_invalid( z );
      }
    }
  }

  if (dbg)    LOG_DEBUG( main_logger, "scorable mgrs: " << t.row << " @B:\ns: " << s << "\nsw: " << sw << "\nne: " << ne );

  // If any of ne, nw, or s is now invalid, bail out
  if ( ! ( s.valid && sw.valid && ne.valid ))
  {
    this->mgrs_valid_latch() = false;
    if (dbg) LOG_DEBUG( main_logger, "scorable mgrs: " << t.row  << " exiting and setting latch" );
    return;
  }

  // We now know there is at least one aligned zone between s, ne, and sw.
  // For each aligned zone, incorporate s's values into the sw/ne bounding box.
  unsigned c=0;
  for (int z=scorable_mgrs::ZONE_BEGIN; z < scorable_mgrs::N_ZONES; ++z )
  {
    if ( ! s.entry_valid[z] ) continue;
    int this_zone = s.zone[z];
    int z_in_sw = sw.find_zone( this_zone );
    int z_in_ne = ne.find_zone( this_zone );
    if ( ( z_in_sw == scorable_mgrs::N_ZONES ) ||
         ( z_in_ne == scorable_mgrs::N_ZONES ) )
    {
      continue;
    }

    // Whew.  We have now finally arrived at the following:
    // - s.zone[z], sw.zone[z_in_sw], ne.zone[z_in_ne] are all the same zone.
    // - All these are valid entries.
    // We can now adjust ne / sw to include s.

    ++c;

    sw.northing[ z_in_sw ] = min( sw.northing[ z_in_sw ], s.northing[ z ] );
    sw.easting[ z_in_sw ] = min( sw.easting[ z_in_sw ], s.easting[ z ] );
    ne.northing[ z_in_ne ] = max( ne.northing[ z_in_ne ], s.northing[ z ] );
    ne.easting[ z_in_ne ] = max( ne.easting[ z_in_ne ], s.easting[ z ] );
  }

  if (c == 0) throw runtime_error( "Failed to update any zones?" );

  this->ne_point() = ne;
  this->sw_point() = sw;

  if (dbg) LOG_DEBUG( main_logger, "scorable mgrs: " << t.row  << " exiting okay" );
  // all done!
}
#endif

void
quickfilter_box_type
::add_image_box( const track_handle_type& t,
                 vgl_box_2d<double> box )
{
  this->Track.set_cursor( t.row );

  // if the coord_system is unset, assume this is the first time we're called
  // bootstrap to the box
  if (this->coord_system() == COORD_NONE)
  {
    this->coord_system() = COORD_IMG;
    this->img_box() = box;
    return;
  }
  if (this->coord_system() != COORD_IMG) throw runtime_error( "Logic error: image called on MGRS box?" );

  // compared to MGRS, adding an image box is very simple
  box.add( this->img_box() );
  this->img_box() = box;
}

#ifdef KWANT_ENABLE_MGRS
double
quickfilter_box_type
::mgrs_box_intersect( const track_handle_type& t1,
                      const track_handle_type& t2 )
{
  static quickfilter_box_type other;
  this->Track.set_cursor( t1.row );
  other.Track.set_cursor( t2.row );
  bool dbg = check_debug( t1, t2 );

  // are both t1 and t2 MGRS boxes?
  if ( (this->coord_system() != COORD_MGRS) || (other.coord_system() != COORD_MGRS) )
  {
    if (dbg) LOG_DEBUG( main_logger, "mgrs_box_intersect: t1 coord system " << this->coord_system() <<
                        "; t2 coord system " << other.coord_system() );
    return -1.0;
  }

  const scorable_mgrs t1_sw( this->sw_point() ), t1_ne( this->ne_point() ),
    t2_sw( other.sw_point() ), t2_ne( other.ne_point() );

  bool okay = t1_sw.valid && t1_ne.valid && t2_sw.valid && t2_ne.valid;
  if ( ! okay )
  {
    if (dbg) LOG_DEBUG( main_logger, "mgrs_box_intersect: valid corners: " << t1_sw.valid << " " << t1_ne.valid <<
                        t2_sw.valid << " " << t2_ne.valid );
    return -1.0;
  }

  // If we get this far, we try to a valid statement
  // about the two tracks based on their quickfilter boxes.

  for (int z=scorable_mgrs::ZONE_BEGIN; z < scorable_mgrs::N_ZONES; ++z )
  {
    if ( ! t1_sw.entry_valid[z] ) continue;
    int t1_sw_zone = t1_sw.zone[z];
    int t1_ne_index = t1_ne.find_zone( t1_sw_zone );
    int t2_sw_index = t2_sw.find_zone( t1_sw_zone );
    int t2_ne_index = t2_ne.find_zone( t1_sw_zone );
    if ( ( t1_ne_index == scorable_mgrs::N_ZONES ) ||
         ( t2_sw_index == scorable_mgrs::N_ZONES ) ||
         ( t2_ne_index == scorable_mgrs::N_ZONES ) )
    {
      if (dbg) LOG_DEBUG( main_logger, "mgrs_box_intersect: zones: index " << z << " is " << t1_sw_zone << "; " <<
                          t1_ne_index << " " << t2_sw_index << " " << t2_ne_index );
      continue;
    }

    // okay!  We have all the indices of all the points to construct
    // the boxes.  In theory, this could happen twice, if all the points
    // share the same default and alternate zones.  I assume the results
    // would be the same both times, and if not, I'm not sure how to
    // choose between them.  I guess we could run the check both
    // times and only return a valid result (>=0) if all the results
    // match?  For now, just return the first valid result.

    vgl_box_2d<double> t1_box( t1_sw.easting[ z ], t1_ne.easting[ t1_ne_index ],
                               t1_sw.northing[ z ], t1_ne.northing[ t1_ne_index ] );
    vgl_box_2d<double> t2_box( t2_sw.easting[ t2_sw_index ], t2_ne.easting[ t2_ne_index ],
                               t2_sw.northing[ z ], t2_ne.northing[ t2_ne_index ] );

    vgl_box_2d<double> overlap = vgl_intersection( t1_box, t2_box );
    if (dbg) LOG_DEBUG( main_logger, "mgrs_box_intersect: boxes:\n" << t1_box << "\n" << t2_box << "\n" << overlap );

    return (overlap.is_empty()) ? 0.0 : vgl_area( overlap );
  }

  // if we get this far, we found no overlapping zones; the caller will
  // need to do a frame-by-frame comparison
  return -1.0;
}
#endif

double
quickfilter_box_type
::img_box_intersect( const track_handle_type& t1,
                     const track_handle_type& t2 )
{
  static quickfilter_box_type other;
  this->Track.set_cursor( t1.row );
  other.Track.set_cursor( t2.row );
#ifdef QF_DEBUG
  bool dbg = check_debug( t1, t2 );
#endif

  // are both t1 and t2 image boxes?
  if ( (this->coord_system() != COORD_IMG) || (other.coord_system() != COORD_IMG) )
  {
#ifdef QF_DEBUG
    if (dbg) LOG_DEBUG( main_logger, "img_box_intersect: t1 coord system " << this->coord_system() <<
                        "; t2 coord system " << other.coord_system() );
#endif
    return -1.0;
  }

  // get the boxes
  vgl_box_2d<double> box_1 = this->img_box();
  vgl_box_2d<double> box_2 = other.img_box();
  if (box_1.is_empty() || box_2.is_empty())
  {
#ifdef QF_DEBUG
    if (dbg) LOG_DEBUG( main_logger, "img_box_intersect: box1/box2 empty:\n" << box_1 << "\n" << box_2 );
#endif
    return -1.0;
  }

  return vgl_area( vgl_intersection( box_1, box_2 ));
}

double
quickfilter_box_type
::quickfilter_check( const track_handle_type& t1,
                     const track_handle_type& t2,
                     bool use_radial_overlap )
{
#ifdef KWANT_ENABLE_MGRS
  return
    (use_radial_overlap)
    ? this->mgrs_box_intersect( t1, t2 )
    : this->img_box_intersect( t1, t2 );
#else
  if (use_radial_overlap)
  {
    throw std::runtime_error("Use of radial overlap without MGRS support");
  }
  else
  {
    return this->img_box_intersect( t1, t2 );
  }
#endif
}


void
quickfilter_box_type
::add_quickfilter_boxes( const track_handle_list_type& t,
                         const phase1_parameters& params )
{
  for (size_t i=0; i<t.size(); ++i)
  {
    add_quickfilter_box( t[i], params );
  }
}

} // ...kwant
} // ...kwiver

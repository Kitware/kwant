/*ckwg +5
 * Copyright 2010-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "phase1_parameters.h"

#include <sstream>
#include <limits>
#include <stdexcept>
#include <algorithm>

#include <vgl/vgl_intersection.h>
#include <vgl/vgl_convex.h>
#include <vgl/vgl_area.h>

#include <vul/vul_reg_exp.h>
#include <vul/vul_sprintf.h>

#ifdef KWANT_ENABLE_MGRS
#include <track_oracle/file_formats/track_scorable_mgrs/track_scorable_mgrs.h>
#endif

#include <scoring_framework/matching_args_type.h>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::istringstream;
using std::make_pair;
using std::map;
using std::numeric_limits;
using std::pair;
using std::runtime_error;
using std::string;
using std::vector;

using kwiver::track_oracle::field_handle_type;
using kwiver::track_oracle::frame_handle_type;
using kwiver::track_oracle::frame_handle_list_type;
using kwiver::track_oracle::track_handle_type;
using kwiver::track_oracle::track_handle_list_type;
using kwiver::track_oracle::track_oracle_core;

namespace // anon
{

using namespace ::kwiver::kwant;

#ifdef KWANT_ENABLE_MGRS
bool
create_geo_poly( const vector< scorable_mgrs >& corners,
                 vector< mgrs_aoi >& aoi_list )
{
  for (size_t i=0; i < corners.size(); ++i)
  {
    if ( ! corners[i].valid )
    {
      LOG_ERROR( main_logger, "GEO AOI corner invalid?" );
      return false;
    }
  }

  // this is a n-ways variant of scorable_mgrs::align_zones.  We want up to N_ZONES
  // vectors; each vector should have 4 entries (one for NW,NE,SE,SW); each entry
  // is a zone index I such that for each corner, zone[I] is the same zone and thus
  // the UTM coordinates are commensurate.  Any vector with fewer than 4 entries
  // is invalid (no single zone in either default or alt holds all four corners.)
  // If there are no valid vectors, log and error and return false.

  typedef pair< size_t, unsigned int> corner_zone_pair;  // first = corner (0..3); second = zone
  typedef map< int, vector< corner_zone_pair > > zone_to_corner_map_type;
  typedef map< int, vector< corner_zone_pair > >::const_iterator zone_to_corner_map_cit;

  zone_to_corner_map_type zone_to_corner_map;
  for (size_t i=0; i<corners.size(); ++i)
  {
    const scorable_mgrs& s = corners[i];
    for (size_t j=0; j<scorable_mgrs::N_ZONES; ++j)
    {
      if ( ! s.entry_valid[j] ) continue;
      zone_to_corner_map[ s.zone[j] ].push_back( make_pair( i, j ));
    }
  }

  aoi_list.clear();
  for (zone_to_corner_map_cit i=zone_to_corner_map.begin(); i != zone_to_corner_map.end(); ++i)
  {
    const vector< corner_zone_pair >& v = i->second;
    if (v.size() != 4) continue;

    // create the vector of (easting, northing) points
    vector< vgl_point_2d<double > > pts;
    for (size_t j=0; j<v.size(); ++j)
    {
      const corner_zone_pair& czp = v[j];
      const scorable_mgrs& s = corners[ czp.first ];
      if (! s.entry_valid[czp.second] )
      {
        throw runtime_error( "Invalid MGRS comparison constructing geo AOI" );
      }
      pts.push_back( vgl_point_2d<double>( s.easting[ czp.second ], s.northing[ czp.second ] ));
      LOG_DEBUG( main_logger, "adding point " << vul_sprintf("%3.9f, %3.9f", pts.back().x(), pts.back().y() ) );
    }

    // convert to a convex hull

    mgrs_aoi a;
    a.zone = i->first;
    a.aoi = vgl_convex_hull( pts );
    LOG_INFO( main_logger, "GEO AOI area " << vgl_area_signed( a.aoi ) );
    aoi_list.push_back( a );
  }

  if ( aoi_list.empty() )
  {
    LOG_ERROR( main_logger, "geo AOI failed to generate any UTM boxes; perhaps no single zone holds all corners?" );
    return false;
  }

  return true;
}
#endif
  
} // anon

namespace kwiver {
namespace kwant {

bool
phase1_parameters
::processMatchingArgs( const matching_args_type& m )
{
  this->radial_overlap = m.radial_overlap();
  this->min_bound_matching_area = m.min_bound_area();
  this->min_frames_policy = m.min_frames_policy;
  this->pass_all_nonzero_overlaps = m.pass_nonzero_overlaps();
  this->frame_alignment_time_window_usecs = m.frame_alignment_secs() * 1e6;
  this->iou =
    m.iou.set()
    ? m.iou()
    : -1;

  if (m.aoi_string.set())
  {
    if ( ! this->setAOI( m.aoi_string() ))
    {
      return false;
    }
  }

  // if the min-pcent-gt-ct arg is set, parse it
  if ( m.min_pcent_gt_ct.set() )
  {
    if ( ! m.parse_min_pcent_gt_ct(
           this->min_pcent_overlap_gt_ct,
           this->debug_min_pcent_overlap_gt_ct ) )
    {
      return false;
    }
  }

  return true;
}



void
phase1_parameters
::setAOI(vgl_box_2d<double> bbox, bool inclusive)
{
  this->b_aoi = bbox;
  this->aoiInclusive = inclusive;
}

bool
phase1_parameters
::setAOI( const string& aoi_string )
{
  //
  // the AOI string is either:
  //
  // pixel: 'WxH+x+y' , e.g. 240x191+1500+1200
  //
  // or
  //
  // lon/lat: 'NW_lon,NW_lat:SE_lon,SE_lat' , e.g -73.8,42.15:-72.9,41.9
  //
  // or
  //
  // four lon points, e.g -73.8,42.15:-72.9,41.9:-73.8,42:,-71.3,41
  //
  // or (TODO):
  //
  // 'G' or 'C' to auto-compute the AOI from the convex hull of the (geo, computed)

  string int_re( "([0-9]+)" );
  string dbl_re( "([-+]?[0-9\\.]+)" );
  string geo_aoi_str( dbl_re+","+dbl_re );

  vul_reg_exp pixel_aoi_re ( string(int_re + "x" + int_re + "\\+" + int_re + "\\+" + int_re ).c_str() );
  vul_reg_exp geo_2_corner_aoi_re( string(dbl_re + "," + dbl_re + ":" + dbl_re + "," + dbl_re ).c_str() );
  vul_reg_exp geo_4_corner_aoi_re( string(dbl_re + "," + dbl_re + ":" + dbl_re + "," + dbl_re + ":" +
                                              dbl_re + "," + dbl_re + ":" + dbl_re + "," + dbl_re ).c_str() );

  if ( pixel_aoi_re.find( aoi_string ))
  {
    vgl_box_2d<double> aoi;

    double x,y,w,h;
    x = y = w = h = -1;

    istringstream iss( pixel_aoi_re.match(1)+" "+pixel_aoi_re.match(2)+" "+pixel_aoi_re.match(3)+" "+pixel_aoi_re.match(4) );
    if ( ! (iss >> w >> h >> x >> y))
    {
      LOG_ERROR( main_logger, "pixel AOI string '" << aoi_string << "': couldn't parse 4 doubles from '" << iss.str() << "'" );
      return false;
    }

    if(x != -1 || y != -1 || w != -1 || h != -1)
    {
      aoi.set_min_x(x);
      aoi.set_min_y(y);
      aoi.set_max_x(x+w);
      aoi.set_max_y(y+h);
      LOG_INFO( main_logger, "pixel AOI string '" << aoi_string << " gives AOI of " << aoi );
    }
    this->setAOI( aoi, /* inclusive = */ true );
  } // ...if pixel AOI

#ifdef KWANT_ENABLE_MGRS
  else if ( geo_4_corner_aoi_re.find( aoi_string ))
  {
    vector< scorable_mgrs > corners;
    size_t c = 0; // matches start at 1
    for ( size_t i = 0; i<4; ++i )
    {
      istringstream iss( geo_4_corner_aoi_re.match( c+1 )+" "+geo_4_corner_aoi_re.match( c+2 ));
      c += 2; // can't use '++c' in line above, undefined
      double lon, lat;
      if ( ! (iss >> lon >> lat ))
      {
        LOG_ERROR( main_logger, "GEO 4-corner AOI string '" << aoi_string << "': couldn't parse lon/lat pair " << i << " from '" << iss.str() << "'" );
        return false;
      }
      corners.push_back( scorable_mgrs( geographic::geo_coords( lat, lon )));
    }

    LOG_DEBUG( main_logger,"4-corner");
    return create_geo_poly( corners, this->mgrs_aoi_list );
  } // .. if geo 4-corner AOI

  else if ( geo_2_corner_aoi_re.find( aoi_string ))
  {
    double NW_lon, NW_lat, SE_lon, SE_lat;
    istringstream iss( geo_2_corner_aoi_re.match(1)+" "+geo_2_corner_aoi_re.match(2)+" "+geo_2_corner_aoi_re.match(3)+" "+geo_2_corner_aoi_re.match(4) );
    if ( ! (iss >> NW_lon >> NW_lat >> SE_lon >> SE_lat ))
    {
      LOG_ERROR( main_logger, "GEO AOI string '" << aoi_string << "': couldn't parse 4 doubles from '" << iss.str() << "'" );
      return false;
    }

    // check for geographic compatibility across all four corners
    vector< scorable_mgrs > corners;
    corners.push_back( scorable_mgrs( geographic::geo_coords( NW_lat, NW_lon )));  // NW corner
    corners.push_back( scorable_mgrs( geographic::geo_coords( NW_lat, SE_lon )));  // NE corner
    corners.push_back( scorable_mgrs( geographic::geo_coords( SE_lat, SE_lon )));  // SE corner
    corners.push_back( scorable_mgrs( geographic::geo_coords( SE_lat, NW_lon )));  // SW corner
    LOG_DEBUG( main_logger,"2-corner");

    return create_geo_poly( corners, this->mgrs_aoi_list );
  } // .. if geo 2-corner AOI
#endif
  else
  {
    LOG_ERROR( main_logger, "AOI string '" << aoi_string << "' failed to parse as either pixel or geo AOI" );
    return false;
  }

  return true;
}

void
phase1_parameters
::set_frame_window( ts_type f0, ts_type f1 )
{
  this->frame_window = phase1_frame_window( f0, f1 );
}

phase1_parameters::AOI_STATUS
phase1_parameters
::get_aoi_status() const
{
  if (this->radial_overlap < 0)
  {
    // pixel or nothing
    return
      this->b_aoi.is_empty()
      ? NO_AOI_USED
      : PIXEL_AOI;
  }
  else
  {
    // geo or nothing
    return
      this->mgrs_aoi_list.empty()
      ? NO_AOI_USED
      : GEO_AOI;
  }
}

bool
phase1_parameters
::frame_within_pixel_aoi( const frame_handle_type& fh ) const
{
  static scorable_track_type track;
  field_handle_type bbox_field = track.bounding_box.get_field_handle();

  // default to false because this method is called only if an AOI is defined
  bool frame_in_aoi = false;
  if ( track_oracle_core::field_has_row( fh.row, bbox_field ))
  {
    vgl_box_2d<double> box = track[ fh ].bounding_box();
    if (this->expand_bbox)
    {
      // only expand if unexpanded is in AOI
      vgl_box_2d<double> tmp = vgl_intersection( box, this->b_aoi );
      if ( ! tmp.is_empty() )
      {
        box.expand_about_centroid( this->bbox_expansion );
      }
    }

    vgl_box_2d<double> overlap = vgl_intersection( box, this->b_aoi );
    // see discussion in score_phase1.cxx
    frame_in_aoi = ( (! overlap.is_empty()) == this->aoiInclusive );
  }
  return frame_in_aoi;
}

bool
phase1_parameters
::frame_within_geo_aoi( const frame_handle_type& fh ) const
{
#ifndef KWANT_ENABLE_MGRS
  throw std::runtime_error( "MGRS method called before MGRS code ported over" );
#else
  static scorable_track_type dbg;
  static track_scorable_mgrs_type track;
  field_handle_type mgrs_field = track.mgrs.get_field_handle();

  // default to false because this method is called only if an AOI is defined
  bool frame_in_aoi = false;
  if ( track_oracle::field_has_row( fh.row, mgrs_field ))
  {
    const scorable_mgrs& m1 = track[ fh ].mgrs();
    // find a compatible zone
    int z1 = scorable_mgrs::N_ZONES;
    int aoi_z = scorable_mgrs::N_ZONES;
    for (size_t i=0; i<this->mgrs_aoi_list.size(); ++i)
    {
      for (int p1 = scorable_mgrs::ZONE_BEGIN; p1 < scorable_mgrs::N_ZONES; ++p1)
      {
        if (this->mgrs_aoi_list[i].zone == m1.zone[p1])
        {
          z1 = p1;
          aoi_z = i;
        }
      }
    }

    if ( (z1 != scorable_mgrs::N_ZONES) && (aoi_z != scorable_mgrs::N_ZONES))
    {
      const vgl_polygon<double>& box = this->mgrs_aoi_list[ aoi_z ].aoi;
      if ( !m1.entry_valid[ z1 ])
      {
        throw runtime_error( "Invalid MGRS comparison checking frame_within_geo_aoi" );
      }
      frame_in_aoi = box.contains( m1.easting[ z1 ], m1.northing[ z1 ] );
    }

  }
  return frame_in_aoi;
#endif
}


pair< ts_type, ts_type >
phase1_parameters
::filter_track_list_on_aoi( const track_handle_list_type& in,
                            track_handle_list_type& out )
{
  scorable_track_type track;
  ts_type min_ts = numeric_limits<ts_type>::max();
  ts_type max_ts = numeric_limits<ts_type>::min();

  AOI_STATUS aoi_status = this->get_aoi_status();

  for (unsigned i=0; i<in.size(); ++i)
  {
    bool keep_track = false;
    unsigned int frames_in_aoi = 0;

    // if both the AOI and frame window are empty / unset, keep track and quick exit
    if ((aoi_status == NO_AOI_USED) && ( ! this->frame_window.is_set ) )
    {
      keep_track = true;
      frames_in_aoi += track_oracle_core::get_n_frames( in[i] );
    }
    else
    {
      // otherwise, must examine each frame to see if it's in the AOI or frame window

      frame_handle_list_type frames = track_oracle_core::get_frames( in[i] );
      for (unsigned j=0; j<frames.size(); ++j)
      {
        const frame_handle_type& f = frames[j];
        // these are updated to the ts of the frame only if the frame
        // is in the aoi; otherwise they stay at +/- infinity.
        ts_type frame_min_ts = numeric_limits<ts_type>::max();
        ts_type frame_max_ts = numeric_limits<ts_type>::min();

        // frame window check
        bool frame_in_time_window = true; // default to true if no time window set
        if ( this->frame_window.is_set )
        {
          ts_type this_ts = track[ f ].timestamp_usecs();
          this_ts /= static_cast<ts_type>(1.0e6); // hack!
          frame_in_time_window = ( this->frame_window.f0 <= this_ts ) && ( this_ts <= this->frame_window.f1 );
        }

        // AOI check
        bool frame_in_aoi = true; // default to true if no AOI used
        switch (aoi_status)
        {
        case PIXEL_AOI:
          frame_in_aoi = this->frame_within_pixel_aoi( f );
          break;
        case GEO_AOI:
          frame_in_aoi = this->frame_within_geo_aoi( f );
          break;
        default:
          frame_in_aoi = true;
          break;
        }

        // update frame-matched and timestamps
        if ( frame_in_aoi )
        {
          ++frames_in_aoi;
          track[ f ].frame_has_been_matched() = IN_AOI_UNMATCHED;
          ts_type frame_ts = track[ f ].timestamp_usecs();
          if (frame_ts < frame_min_ts) frame_min_ts = frame_ts;
          if (frame_max_ts < frame_ts) frame_max_ts = frame_ts;
        }
        else
        {
          track[ f ].frame_has_been_matched() = OUTSIDE_AOI;
        }

        // keep the track if any frame is both within the time window and inside the AOI
        if ( frame_in_time_window && frame_in_aoi )
        {
          keep_track = true;
          if ( frame_min_ts < min_ts ) min_ts = frame_min_ts;
          if ( max_ts < frame_max_ts ) max_ts = frame_max_ts;
        }

      } // ... all frames
    } // ... AOI is defined

    track( in[i] ).frames_in_aoi() = frames_in_aoi;

    if ( keep_track )
    {
      out.push_back( in[i] );
    }
  } // ...for all input tracks

  // all done!
  return make_pair( min_ts, max_ts );
}

} // ...kwant
} // ...kwiver

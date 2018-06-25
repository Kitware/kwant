/*ckwg +5
 * Copyright 2010-2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "score_phase1.h"

#include <cstdlib>
#include <cmath>
#include <limits>
#include <fstream>
#include <stdexcept>
#include <typeinfo>

#include <vgl/vgl_area.h>
#include <vgl/vgl_box_2d.h>
#include <vgl/vgl_intersection.h>

#include <vul/vul_timer.h>

#include <track_oracle/core/track_oracle_core.h>
#include <track_oracle/data_terms/data_terms.h>
#include <track_oracle/core/state_flags.h>
#ifdef KWANT_ENABLE_MGRS
#include <track_oracle/file_formats/track_scorable_mgrs/track_scorable_mgrs.h>
#endif
#include <track_oracle/aries_interface/aries_interface.h>

#include <scoring_framework/quickfilter_box.h>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::endl;
using std::make_pair;
using std::map;
using std::max;
using std::min;
using std::numeric_limits;
using std::ofstream;
using std::ostream;
using std::ostringstream;
using std::pair;
using std::runtime_error;
using std::sort;
using std::sqrt;
using std::string;
using std::vector;

using kwiver::track_oracle::field_handle_type;
using kwiver::track_oracle::track_handle_type;
using kwiver::track_oracle::track_handle_list_type;
using kwiver::track_oracle::frame_handle_type;
using kwiver::track_oracle::frame_handle_list_type;
using kwiver::track_oracle::track_oracle_core;
using kwiver::track_oracle::track_field;
using kwiver::track_oracle::element_descriptor;
using kwiver::track_oracle::oracle_entry_handle_type;
using kwiver::track_oracle::descriptor_overlap_type;
using kwiver::track_oracle::descriptor_event_label_type;
using kwiver::track_oracle::single_event_label_type;
using kwiver::track_oracle::aries_interface;

#undef P1_DEBUG
//#define P1_DEBUG

namespace // anonymous
{

using namespace ::kwiver::kwant;

static const ts_type INVALID_TIMESTAMP = static_cast<ts_type>( -1 );


ts_type
ts( frame_handle_type frame_id )
{
  static scorable_track_type local_track_view;
  return local_track_view[ frame_id ].timestamp_usecs();
}

// return the (min, max) timestamp in the list of tracks.

pair< ts_type, ts_type >
track_list_time_bounds( const track_handle_list_type& t )
{
  scorable_track_type track;
  pair< ts_type, ts_type > ret;
  ret.first = INVALID_TIMESTAMP;
  ret.second = INVALID_TIMESTAMP;

  bool first_time = true;
  for (unsigned i=0; i<t.size(); ++i)
  {
    frame_handle_list_type frames = track_oracle_core::get_frames( t[i] );
    for (unsigned j=0; j<frames.size(); ++j)
    {
      ts_type frame_ts = ts( frames[j] );
      if ( first_time )
      {
        ret.first = frame_ts;
        ret.second = frame_ts;
        first_time = false;
      }
      else
      {
        if (frame_ts < ret.first) ret.first = frame_ts;
        if (frame_ts > ret.second) ret.second = frame_ts;
      }
    }
  }
  return ret;
}

// run some basic sanity checks on the two track lists:
// do they contain frames?  Do the timestamps overlap AT ALL?

bool
sanity_check_track_list_timestamps( const track_handle_list_type& a,
                                    const track_handle_list_type& b )
{
  pair< ts_type, ts_type > ts_a, ts_b;
  ts_a = track_list_time_bounds( a );
  ts_b = track_list_time_bounds( b );

  if ( (ts_a.first == ts_a.second) && (ts_a.first == INVALID_TIMESTAMP ))
  {
    LOG_ERROR( main_logger, "Sanity check: first track list has no frames" );
    return false;
  }
  if ( (ts_b.first == ts_b.second) && (ts_b.first == INVALID_TIMESTAMP ))
  {
    LOG_ERROR( main_logger, "Sanity check: second track list has no frames" );
    return false;
  }

  if ( (ts_a.second < ts_b.first) || (ts_b.second < ts_a.first ))
  {
    LOG_ERROR( main_logger, "Sanity check: track list timestamps do not overlap: "
               << "first set: [ " << ts_a.first << " : " << ts_a.second << "]; "
               << "second set: [ " << ts_b.first << " : " << ts_b.second << "]" );
    return false;
  }

  // all is well!
  return true;
}

bool
test_if_overlap_passes_filters( const track2track_frame_overlap_record& overlap,
                                const phase1_parameters& params )
{
  //
  // process spatial overlaps.
  //
  // If either of min_pcent_overlap_gt_ct is >= 0 (i.e. not don't-care), then
  // compute the checks on percentages of overlap of both the ground-truth and
  // computed-track boxes.  The absolute min-bound-matching-area parameter is
  // ignored.  If both of min_pcent_overlap_gt_ct are <0, then use the
  // min-bound-matching-area parameter.
  //

  static scorable_track_type local_track_view;
  bool spatial_overlap_exists = false;

  bool use_min_pcent_gt = (params.min_pcent_overlap_gt_ct.first >= 0.0);
  bool use_min_pcent_ct = (params.min_pcent_overlap_gt_ct.second >= 0.0);

  bool use_iou = (params.iou != -1.0 );

  if ( use_min_pcent_gt || use_min_pcent_ct )
  {
    double pcent_gt_overlap = 100.0 * overlap.overlap_area / overlap.truth_area;
    double pcent_ct_overlap = 100.0 * overlap.overlap_area / overlap.computed_area;

    bool pcent_gt_matched = ( ( ! use_min_pcent_gt ) ||
                              (pcent_gt_overlap >= params.min_pcent_overlap_gt_ct.first) );
    bool pcent_ct_matched = ( ( ! use_min_pcent_ct ) ||
                              (pcent_ct_overlap >= params.min_pcent_overlap_gt_ct.second) );

    spatial_overlap_exists = pcent_gt_matched && pcent_ct_matched;

    if (params.debug_min_pcent_overlap_gt_ct)
    {
      static bool first_time = true;
      if (first_time)
      {
        LOG_DEBUG( main_logger, "Note that some parameters (e.g. areas) are only initialized if overlap area is > 0" );
        first_time = false;
      }
      LOG_DEBUG( main_logger, "gt/ct ts " << overlap.truth_frame << " / " << overlap.computed_frame
                 << " ; gt/ct areas " << overlap.truth_area << " / " << overlap.computed_area
                 << " ; overlap area " << overlap.overlap_area
                 << " ; gt/ct %ages " << pcent_gt_overlap << " / " << pcent_ct_overlap
                 << " ; final overlap decision: " << spatial_overlap_exists );

      LOG_DEBUG( main_logger, "... GT was " << local_track_view.bounding_box( overlap.truth_frame.row ) << " @ " <<
                 local_track_view.timestamp_frame( overlap.truth_frame.row) << " / " <<
                 local_track_view.timestamp_usecs( overlap.truth_frame.row) );
      LOG_DEBUG( main_logger, "... CT was " << local_track_view.bounding_box( overlap.computed_frame.row ) << " @ " <<
                 local_track_view.timestamp_frame( overlap.computed_frame.row) << " / " <<
                 local_track_view.timestamp_usecs( overlap.computed_frame.row) );

    }
  }
  else if (use_iou)
  {
    double i = overlap.overlap_area;
    double u = overlap.truth_area + overlap.computed_area - i;
    spatial_overlap_exists =
      (u > 0)
      ? i/u >= params.iou
      : false;
  }
  else
  {
    if (overlap.overlap_area > params.min_bound_matching_area)
    {
      spatial_overlap_exists = true;
    }
#if P1_DEBUG
    LOG_DEBUG( main_logger, "test-if-overlap-passes: " << overlap.overlap_area << " vs " << params.min_bound_matching_area << ": " << spatial_overlap_exists );
#endif
  }

  return spatial_overlap_exists;
}

} // ...anon namespace

namespace kwiver {
namespace kwant {

void
debug_dump_first_n_frames( const string& tag,
                           const frame_handle_list_type& f,
                           size_t n_requested = 10 )
{
  track_field<unsigned> fn("frame_number");
  track_field<ts_type> timestamp("timestamp_usecs");
  size_t n = (f.size() < n_requested) ? f.size() : n_requested;
  LOG_INFO( main_logger, tag );
  for (size_t i=0; i<n; ++i)
  {
    LOG_INFO( main_logger, "index " << i << " is " << fn( f[i].row ) << " ts " << timestamp( f[i].row ) << " h " << f[i].row );
  }
}


bool
timestamp_compare( frame_handle_type lhs,
                   frame_handle_type rhs )
{
  track_field<ts_type> timestamp("timestamp_usecs");
  ts_type lhs_ts = timestamp( lhs.row );
  ts_type rhs_ts = timestamp( rhs.row );
  return lhs_ts < rhs_ts;
}

bool
compare_frame_lists( const vector< pair< frame_handle_type, frame_handle_type> >& f1,
                     const vector< pair< frame_handle_type, frame_handle_type> >& f2 )
{
  if (f1.size() != f2.size() )
  {
    LOG_ERROR( main_logger, "Frame list check: sizes differ: " << f1.size() << " vs " << f2.size() );
    return false;
  }
  map< pair< frame_handle_type, frame_handle_type>, int > count_map;
  for (unsigned i=0; i<f1.size(); ++i) count_map[ f1[i] ] += 0x01;
  for (unsigned i=0; i<f1.size(); ++i) count_map[ f1[i] ] += 0x02;
  bool okay = true;
  for (map< pair< frame_handle_type, frame_handle_type>, int >::const_iterator i=count_map.begin();
       i != count_map.end();
       ++i)
  {
    if (i->second != 3)
    {
      LOG_ERROR( main_logger, "Frame list check: entry with flag " << i->second );
      okay = false;
    }
  }
  return okay;
}


frame_handle_list_type
sort_frames_by_field( track_handle_type track_id, const string& name )
{
  string key = "sort_frames_by_"+name;
  field_handle_type sorted_frames_field = track_oracle_core::lookup_by_name( key );
  if ( sorted_frames_field == kwiver::track_oracle::INVALID_FIELD_HANDLE )
  {
    sorted_frames_field = track_oracle_core::create_element< frame_handle_list_type >(
      element_descriptor(
        key,
        "sorted frames of "+name,
        typeid( static_cast< frame_handle_list_type* >(0) ).name(),
        element_descriptor::SYSTEM ));
  }
  if ( ! track_oracle_core::field_has_row( track_id.row, sorted_frames_field ))
  {
    frame_handle_list_type frames = track_oracle_core::get_frames( track_id );
    sort( frames.begin(), frames.end(), timestamp_compare );
    track_oracle_core::get_field<frame_handle_list_type>( track_id.row, sorted_frames_field ) = frames;
  }
  return track_oracle_core::get_field<frame_handle_list_type>( track_id.row, sorted_frames_field );
}


bool
track2track_score
::move_a_up_to_b( unsigned& index,
                  const frame_handle_list_type& lagging_list,
                  unsigned fixed_index,
                  const frame_handle_list_type& fixed_list,
                  double match_window )
{
  if (fixed_list.empty()) return false;

  ts_type target_ts = ts( fixed_list[fixed_index] );
#ifdef P1_DEBUG
  LOG_INFO( main_logger, " ...a2b: target " << target_ts << "; index " << index );
#endif

  unsigned n = lagging_list.size();
  while ( index < n )
  {
    ts_type t = ts( lagging_list[index] );
    ts_type diff = (t > target_ts) ? t-target_ts : target_ts-t;
#ifdef P1_DEBUG
    LOG_INFO( main_logger, "...... index " << index << " vs n " << n << "  index-t " << t << ": diff " << diff << " (mw " << match_window << ")" );
#endif
    if (diff < match_window)
    {
#ifdef P1_DEBUG
      LOG_INFO( main_logger, "...a2b exiting at " << index );
#endif
      return true;
    }
    else
    {
      // the assertion entering this routine was that lagging (aka t) < target.
      // If we're here, then if t > target, and we're out even taking the
      // match window into account, then we've overshot.  If we've overshot, we
      // should exit false.
      if ( t > target_ts )
      {
#ifdef P1_DEBUG
        LOG_INFO( main_logger, "...a2b overshot; exiting at " << index );
#endif
        return false;
      }
      ++index;
    }
  }
#ifdef P1_DEBUG
  LOG_INFO( main_logger, "...a2b exiting false" );
#endif
  return false;
}

bool
track2track_score
::within_window( const frame_handle_list_type& f1,
                 const frame_handle_list_type& f2,
                 unsigned f1_ptr,
                 unsigned f2_ptr,
                 double match_window )
{
  if (f1_ptr >= f1.size()) return false;
  if (f2_ptr >= f2.size()) return false;

  ts_type f1_ts = ts( f1[ f1_ptr ] );
  ts_type f2_ts = ts( f2[ f2_ptr ] );
  // careful about unsigned types
  if (f1_ts < f2_ts)
  {
    return (f2_ts - f1_ts) < match_window;
  }
  else
  {
    return (f1_ts - f2_ts) < match_window;
  }
}

vector< pair< frame_handle_type, frame_handle_type > >
track2track_score
::align_frames( const frame_handle_list_type& f1,
                const frame_handle_list_type& f2,
                double match_window )
{
#ifdef P1_DEBUG
  track_field<unsigned> fn("frame_number");
  debug_dump_first_n_frames( "align frames: f1", f1 );
  debug_dump_first_n_frames( "align frames: f2", f2 );
#endif

  vector< pair< frame_handle_type, frame_handle_type > > ret;
  if ( f1.empty() || f2.empty() ) return ret;

  // assume f1, f2 are sorted by timestamp

  // quick tests for endpoints
  // ...last frame of f1 before first frame of f2?
  if ( ts( f1.back() )+match_window < ts( f2.front() ) )
  {
#ifdef P1_DEBUG
    LOG_INFO( main_logger, "Quick-exit f1 vs f2: ");
    unsigned long long diff = ts( f2.front() ) - (ts(f1.back()) + match_window);
    LOG_INFO( main_logger, ts( f1.back() ) << " mw " << match_window << " " << ts( f2.front() ) << " diff " << diff << "");
#endif
    return ret;
  }
  // ...last frame of f2 before first frame of f1?
  if ( ts( f2.back() )+match_window < ts( f1.front() ) )
  {
#ifdef P1_DEBUG
    LOG_INFO( main_logger, "Quick-exit f2 vs f1: ");
    unsigned long long diff = ts( f1.front()) - (ts(f2.back()) + match_window);
    LOG_INFO( main_logger, ts( f2.back() ) << " mw " << match_window << " " << ts( f1.front() ) << " diff " << diff << "");
#endif
    return ret;
  }

  // "A" and "B" each map to f1 and f2; which maps to which
  // can change at the start of each round.  The constraint
  // is that the timestamp of A is always less than B at
  // the start of each round.
  //
  // values are always stored in ret in (f1, f2) order, helped
  // by the a_is_f1 state variable below.
  //
  // At the start of each round:
  // -- if A or B has run off the end, return.
  // -- if ts(B) < ts(A), swap how A and B are mapped to f1 / f2.
  // Thus, start off each round such that ts(A) < ts(B).
  //
  // If ts(B) - ts(A) is outside the match window:
  // --- A has no match; increment A.
  // If ts(B) - ts(A) is within match window:
  // --- A and B match; increment both.

  bool a_is_f1;  // true if (A==f1, B==f2); false if (A==f2, A==f1)
  size_t f1_index = 0, f2_index = 0;
  size_t n1 = f1.size(), n2 = f2.size();
  while (true)
  {
    // Did we run out of frames?
    if ((f1_index == n1) || (f2_index == n2)) return ret;

#ifdef P1_DEBUG
    {
      unsigned f1_fn = fn( f1[f1_index].row );
      unsigned f2_fn = fn( f2[f2_index].row );
      LOG_DEBUG( main_logger, "Loop top " << f1_index <<  " vs " << f2_index << " (fn " << f1_fn << " / " << f2_fn << " )" );
    }
#endif
    // decide which is A and which is B
    a_is_f1 = (ts(f1[f1_index]) < ts(f2[f2_index]));
    const frame_handle_list_type& fA = (a_is_f1) ? f1 : f2;
    const frame_handle_list_type& fB = (a_is_f1) ? f2 : f1;
    size_t& fA_index = (a_is_f1) ? f1_index : f2_index;
    size_t& fB_index = (a_is_f1) ? f2_index : f1_index;

    // compute timestamp diff
    ts_type diff = ts( fB[fB_index] ) - ts( fA[fA_index] );
#ifdef P1_DEBUG
    LOG_DEBUG( main_logger, "a-is-f1: " << a_is_f1 << " ; diff " << diff << " vs match_window " << match_window );
#endif
    if (diff < match_window)
    {
      // the frames are aligned!  Record the result and increment
      ret.push_back( make_pair( f1[f1_index], f2[f2_index] ));
#ifdef P1_DEBUG
      {
        unsigned fn_1 = fn( f1[f1_index].row );
        unsigned fn_2 = fn( f2[f2_index].row );
        LOG_DEBUG( main_logger, "Match: size now " << ret.size() << " ; was: f1 / f2 " << fn_1 << " / " << fn_2 << "(index " << f1_index << " / " << f2_index << ")" );
      }
#endif
      ++fA_index;
      ++fB_index;
#ifdef P1_DEBUG
      {
        unsigned fn_1 = fn( f1[f1_index].row );
        unsigned fn_2 = fn( f2[f2_index].row );
        LOG_DEBUG( main_logger, "...now  f1 / f2 " << fn_1 << " / " << fn_2  << "(index " << f1_index << " / " << f2_index << ")");
      }
#endif
    }
    else
    {
      // A has no match, increment past it
#ifdef P1_DEBUG
      {
        unsigned fn_1 = fn( f1[f1_index].row );
        unsigned fn_2 = fn( f2[f2_index].row );
        LOG_DEBUG( main_logger, "NO MATCH: was f1 / f2 " << fn_1 << " / " << fn_2  << "(index " << f1_index << " / " << f2_index << ")");
      }
#endif
      ++fA_index;
#ifdef P1_DEBUG
      {
        unsigned fn_1 = fn( f1[f1_index].row );
        unsigned fn_2 = fn( f2[f2_index].row );
        LOG_DEBUG( main_logger, "...now  f1 / f2 " << fn_1 << " / " << fn_2 << "(index " << f1_index << " / " << f2_index << ")" );
      }
#endif
    }
  }

  // shouldn't get here
  LOG_ERROR( main_logger, "Logic error in " << __FILE__ << "::" << __LINE__ << "");
  // hush some compilers
  return ret;
}

#ifdef KWANT_ENABLE_MGRS
track2track_frame_overlap_record
track2track_score
::compute_radial_overlap( frame_handle_type f1, frame_handle_type f2, const phase1_parameters& params )
{
  static track_scorable_mgrs_type local_track_view;

  field_handle_type mgrs_field = local_track_view.mgrs.get_field_handle();
  if ( (! track_oracle_core::field_has_row( f1.row, mgrs_field )) ||
       (! track_oracle_core::field_has_row( f2.row, mgrs_field )) )
  {
    throw runtime_error( "radial overlap requested on frame with no mgrs" );
  }

  scorable_mgrs m1 = local_track_view[ f1 ].mgrs();
  scorable_mgrs m2 = local_track_view[ f2 ].mgrs();

  track2track_frame_overlap_record ret;
  ret.truth_frame = f1;
  ret.computed_frame = f2;
  ret.truth_area = -1.0;
  ret.computed_area = -1.0;
  ret.overlap_area = -1.0;
  ret.center_bottom_distance = -1.0;

  ret.in_aoi = false;

  if ( params.mgrs_aoi_list.empty() )
  {
    ret.in_aoi = true;
  }
  else
  {
    // find a zone containing all of aoi, m1, and m2
    size_t aoi_index = params.mgrs_aoi_list.size();
    vector< int > zone_map = m1.align_zones( m2 );
    int p1, p2;
    int matched_p1 = scorable_mgrs::N_ZONES;
    for (p1 = scorable_mgrs::ZONE_BEGIN; (matched_p1 == scorable_mgrs::N_ZONES) && (p1 < scorable_mgrs::N_ZONES); ++p1)
    {
      p2 = zone_map[ p1 ];
      if (p2 == scorable_mgrs::N_ZONES) continue;

      int zone = m1.zone[ p1 ]; // ...by definition also m2's .zone[p2]; is it in mgrs_aoi_list?
      for (size_t i=0; i<params.mgrs_aoi_list.size(); ++i)
      {
        if ( params.mgrs_aoi_list[i].zone == zone )
        {
          // match found!
          matched_p1 = p1;
          aoi_index = i;
        }
      }
    }

    // if match_found, then aoi_index is the mgrs_aoi index and p1 is the zone_map index
    // The frame pair is considered to be in the aoi if each frame's detection is within
    // the aoi.
    if (matched_p1 != scorable_mgrs::N_ZONES)
    {
      const vgl_polygon<double>& box = params.mgrs_aoi_list[ aoi_index ].aoi;
      if (! (m1.entry_valid[matched_p1] && m2.entry_valid[p2]))
      {
        throw runtime_error( "Invalid MGRS comparison computing radial overlap" );
      }
      ret.in_aoi = ( box.contains( m1.easting[ matched_p1 ], m1.northing[ matched_p1 ] ) &&
                     box.contains( m2.easting[ p2 ], m2.northing[ p2 ] ));
    }
  }

  if (ret.in_aoi)
  {
    ret.centroid_distance = scorable_mgrs::diff( m1, m2 );
  }
  else
  {
    ret.centroid_distance = -1.0;
  }

  return ret;
}
#endif

track2track_frame_overlap_record
track2track_score
::compute_spatial_overlap( frame_handle_type t1, frame_handle_type t2, phase1_parameters const& params )
{
  static scorable_track_type local_track_view;

  typedef vgl_box_2d<double> bbox_type;
  track2track_frame_overlap_record ret;
  ret.truth_frame = t1;
  ret.computed_frame = t2;
  ret.fL_frame_num = local_track_view[ t1 ].timestamp_frame();
  ret.fR_frame_num = local_track_view[ t2 ].timestamp_frame();

  field_handle_type bbox_field = local_track_view.bounding_box.get_field_handle();


  if ( ( ! track_oracle_core::field_has_row( t1.row, bbox_field ) ) ||
       ( ! track_oracle_core::field_has_row( t2.row, bbox_field )))
  {
#ifdef P1_DEBUG
    LOG_INFO( main_logger, "cso: no field/row " << t1 << " , " << bbox_field );
#endif
    return ret;
  }

  bbox_type b1 = local_track_view[ t1 ].bounding_box();
  bbox_type b2 = local_track_view[ t2 ].bounding_box();

  if( params.expand_bbox )
  {
	  b1.expand_about_centroid( params.bbox_expansion );
	  b2.expand_about_centroid( params.bbox_expansion );
  }

  if( params.b_aoi.is_empty() )
  {
    ret.in_aoi = true;
  }
  else
  {
    // Only set to true if BOTH bounding boxes intersect the AOI.

    bbox_type t1i = vgl_intersection( b1, params.b_aoi );
    bbox_type t2i = vgl_intersection( b2, params.b_aoi );
    ret.in_aoi = ( ! t1i.is_empty()) && ( ! t2i.is_empty());
  }

  bbox_type bi = vgl_intersection( b1, b2 );

#ifdef P1_DEBUG
  LOG_INFO( main_logger, "spatial " << t1 << "," << t2 << ": " << b1 << ", " << b2 << ", " << bi );
#endif
  if ( ! bi.is_empty() )
  {
    // it's pretty annoying but some of the code (e.g. score_phase2_aipr.cxx:60)
    // seems to rely on these being set ONLY if there is overlap
    ret.truth_area = vgl_area( b1 );
    ret.computed_area = vgl_area( b2 );
    ret.overlap_area = vgl_area( bi );

    double dx = b1.centroid_x() - b2.centroid_x();
    double d_center_y = b1.centroid_y() - b2.centroid_y();
    double d_bottom_y = b1.max_y() - b2.max_y();

    ret.centroid_distance = sqrt( (dx*dx) + (d_center_y*d_center_y) );
    ret.center_bottom_distance = sqrt( (dx*dx) + (d_bottom_y*d_bottom_y) );

  }
  return ret;
}

void
debug_dump_alignments( const frame_handle_list_type& t,
                       const frame_handle_list_type& c,
                       const vector< pair< frame_handle_type, frame_handle_type> >& alignments,
                       unsigned int t_row,
                       unsigned int c_row )
{
  static scorable_track_type local_track_view;
  static unsigned id = 0;
  ostringstream oss;
  oss << "alignment-" << id++ << ".dat";
  ofstream os( oss.str().c_str());
  if ( ! os )
  {
    LOG_ERROR( main_logger, "Couldn't open " << oss.str() << " for writing");
    return;
  }

  for (unsigned i=0; i<t.size(); ++i)
  {
    os << "t " << t[i].row << " " << local_track_view[ t[i] ].timestamp_usecs() << " " << local_track_view[ t[i] ].bounding_box() << " " << t_row << "\n";
  }
  for (unsigned i=0; i<c.size(); ++i)
  {
    os << "c " << c[i].row << " " << local_track_view[ c[i] ].timestamp_usecs() << " " << local_track_view[ c[i] ].bounding_box() << " " << c_row << "\n";
  }
  for (unsigned i=0; i<alignments.size(); ++i)
  {
    os << "a " << alignments[i].first.row << " " << alignments[i].second.row << "\n";
  }
}

bool
track2track_score
::compute( track_handle_type t, track_handle_type c, phase1_parameters const& params )
{
  this->cached_truth_track = t;
  this->cached_comp_track = c;

  bool use_radial_overlap = (params.radial_overlap >= 0.0);

  //
  // Use the quickfilter boxes if possible
  //
  static quickfilter_box_type qf;
  double qf_check = qf.quickfilter_check( t, c, use_radial_overlap );
  if ( qf_check == 0 )
  {
    this->frame_overlaps.clear();
    return false;
  }

  static scorable_track_type local_track_view;
#ifdef P1_DEBUG
  LOG_INFO( main_logger,"Sorting truth tracks...");
#endif
  frame_handle_list_type t_sorted_frames = sort_frames_by_field( t, "timestamp_usecs" );
#ifdef P1_DEBUG
  LOG_INFO( main_logger,"Sorting computed tracks...");
#endif
  frame_handle_list_type c_sorted_frames = sort_frames_by_field( c, "timestamp_usecs" );

#ifdef P1_DEBUG
  debug_dump_first_n_frames( "t-unsorted", track_oracle_core::get_frames(t) );
  debug_dump_first_n_frames( "t-sorted", t_sorted_frames );
  debug_dump_first_n_frames( "c-unsorted", track_oracle_core::get_frames(c) );
  debug_dump_first_n_frames( "c-unsorted", c_sorted_frames );
#endif

  vector< pair< frame_handle_type, frame_handle_type > > aligned_frames
    = this->align_frames( t_sorted_frames, c_sorted_frames, params.frame_alignment_time_window_usecs );

#ifdef P1_DEBUG
  LOG_INFO( main_logger, "t-sorted / c-sorted / aligned: " << t_sorted_frames.size() << " " << c_sorted_frames.size() << " " << aligned_frames.size() );
  LOG_INFO( main_logger, "Aligned frames: " << aligned_frames.size() << ":" );
#endif

  this->frame_overlaps.clear();

  // revised AOI logic:
  // The overlap statistics are only valid if there is at least
  // one frame_overlap_record with an AOI match:
  // x == overlap.in_aoi, y == params.aoiInclusive
  //
  //      y=T   y=F
  // x=T   T     F
  // x=F   F     T
  //
  // ...in other words, if the in_aoi and params.aoiInclusive values are the same.

  //
  // There are three optional parameters for accepting or rejecting a
  // frame-to-frame overlap:
  //
  // 1) min_pcent_overlap_gt_ct, which accepts or rejects individual
  // frame overlaps based on the amount of overlap expressed as a
  // function of the ratio of the areas of the overlap / ground truth
  // and overlap / computed boxes;
  //
  // 2) the simple min_bound_matching_area parameter, which also
  // accepts or rejects individual frame overlaps if the overlap area
  // is over or under the parameter;
  //
  // Note that (1) and (2) are mutually exclusive, you may either set
  // one or the other. The number of times (1) or (2) is true is the
  // size of the overlap set.  Call this size N. Then the third
  // parameter is
  //
  // 3) the min_matching_frames parameter, which (if set) rejects the
  // entire set of of overlaps if N is too low.
  //
  //
  // Previously, the number of frames reported as overlaps when
  // computing purity and continuity (and any other frame-level
  // statistics) was precisely N.  In other words, if two tracks had
  // (say) 100 frames with non-zero overlap but only (say) 50 frames
  // whose overlap satisfied (1) or (2), then only those 50 frames
  // were passed downstream for computing frame-level statistics.
  //
  // Now, we can optionally decide that once (1-3) are satisfied, all
  // non-zero overlaps will be passed downstream.  In the example above,
  // although only 50 frames passed the filter criteria for accepting or
  // rejecting the track-to-track match, if accepted, all 100 frames
  // with ANY overlap will be passed downstream.
  //

  // First, compute all the overlaps; along the way, count how many pass
  // the per-frame overlap filter

  vector< pair< bool, track2track_frame_overlap_record > > overlaps;
  size_t strong_overlap_count = 0;
  for (size_t i=0; i<aligned_frames.size(); ++i)
  {
#ifdef KWANT_ENABLE_MGRS
    track2track_frame_overlap_record overlap =
      ( use_radial_overlap )
      ? this->compute_radial_overlap( aligned_frames[i].first, aligned_frames[i].second, params )
      : this->compute_spatial_overlap(  aligned_frames[i].first, aligned_frames[i].second, params );
#else
    track2track_frame_overlap_record overlap;
    if (use_radial_overlap)
    {
      throw std::runtime_error( "Radial overlap used without MGRS support" );
    }
    else
    {
      overlap = this->compute_spatial_overlap(  aligned_frames[i].first, aligned_frames[i].second, params );
    }
#endif

    // aoi match must be checked regardless of overlap area
    // (but only if the AOI is defined.)
    bool this_aoi_match = (  params.b_aoi.is_empty() || ( overlap.in_aoi == params.aoiInclusive ) );

    // do not process this overlap if the AOIs do not match
    if ( ! this_aoi_match) continue;

    // never keep empty overlaps (definition of 'empty' depends on overlap method)
    bool overlap_is_empty =
      ( use_radial_overlap )
      ? overlap.centroid_distance == -1.0
      : overlap.overlap_area == 0;
    if ( overlap_is_empty ) continue;

    //
    // process overlaps.  "Strong" overlaps are ones which meet any options
    // the user requested to tighten the overlap criteria, such as --min-pcent-gt-ct
    // or --match-overlap-lower-bound.  A "weak" overlap is a single-pixel overlap.
    // Pass-nonzero-overlaps allows you to filter track-to-track overlaps on
    // strong overlaps, but compute statistics on strong + weak overlaps.  Radial
    // overlap doesn't (yet) have a strong vs. weak distinction, which is why
    // we disallow both radial-overlap and pass-nonzero-overlap to be specified.

    bool overlap_is_strong =
      ( use_radial_overlap )
      ? ( overlap.centroid_distance <= params.radial_overlap )
      : test_if_overlap_passes_filters( overlap, params );

    overlaps.push_back( make_pair( overlap_is_strong, overlap ));
    if ( overlap_is_strong )
    {
      ++strong_overlap_count;
    }
  }

  if (params.min_frames_policy.first)
  {
    if ((params.min_frames_policy.second != 0) &&
        (strong_overlap_count < params.min_frames_policy.second ))
    {
      // we're outta here!
      return false;
    }
  }
  else
  {
    // convert parameter '10%' into 0.10 * length-of-ground-truth
    double d = params.min_frames_policy.second;
    if (d > 0)
    {
      size_t t_length_filter = static_cast< size_t >( t_sorted_frames.size() * d / 100.0 );
      // percentage parameter can never drive the filter length to zero
      if ((t_length_filter == 0) && ( ! t_sorted_frames.empty() ))
      {
        t_length_filter = 1;
      }
      if ( strong_overlap_count < t_length_filter )
      {
        // we're outta here!
        return false;
      }
    } // ...if d > 0
  } // ...min_length_filter is percentage-based

  // ...otherwise, we're in.  Copy out of the overlaps buffer into
  // this object's frame_overlaps vector based on the value of
  // the pass_all_nonzero_overlaps flag, setting flags and computing
  // time ranges and so forth as we go.

  this->overlap_frame_range.first = numeric_limits< ts_type >::max();
  this->overlap_frame_range.second = numeric_limits< ts_type >::min();

  track_field< kwiver::track_oracle::dt::utility::state_flags > track_flags;
  for (size_t i=0; i<overlaps.size(); ++i)
  {
    bool keep_this = params.pass_all_nonzero_overlaps || overlaps[i].first;
    if ( ! keep_this ) continue;

    const track2track_frame_overlap_record& overlap = overlaps[i].second;

    // record that the frames were matched
    local_track_view[ overlap.truth_frame ].frame_has_been_matched() = IN_AOI_MATCHED;
    local_track_view[ overlap.computed_frame ].frame_has_been_matched() = IN_AOI_MATCHED;
    track_flags( overlap.truth_frame.row ).set_flag( "ATTR_SCORING_STATE_MATCHED" );
    track_flags( overlap.computed_frame.row ).set_flag( "ATTR_SCORING_STATE_MATCHED" );


    // update the frame range
    ts_type this_min_ts = min( ts( overlap.truth_frame ), ts( overlap.computed_frame ));
    this->overlap_frame_range.first = min( this_min_ts, this->overlap_frame_range.first );

    ts_type this_max_ts = max( ts( overlap.truth_frame ), ts( overlap.computed_frame ));
    this->overlap_frame_range.second = max( this_max_ts, this->overlap_frame_range.second );

    this->frame_overlaps.push_back( overlap );
  }
  this->spatial_overlap_total_frames = this->frame_overlaps.size();

  return ( ! this->frame_overlaps.empty() );
}


void
track2track_phase1
::compute_all( const track_handle_list_type& t,
               const track_handle_list_type& c )
{
  if ( params.perform_sanity_checks )
  {
    // c may be empty if the tracker missed everything; in that case, don't throw an error
    // t may also be empty if we're e.g. scoring events (say, PersonWalking), and the truth
    // set has no Walking events, but the computed set does
    if ( (! t.empty() ) && (! c.empty() ) && (! sanity_check_track_list_timestamps( t, c )))
    {
      LOG_ERROR( main_logger, "*\n*\n*\n"
                 << "* The set of aligned frames between ground-truth and computed tracks is empty.\n"
                 << "* There are three possibilities:\n"
                 << "* 1) your ground-truth and and computed data are truly misaligned\n"
                 << "* 2) the frame alignment parameter is set incorrectly,\n"
                 << "* 3) there is an error in the timestamp_usecs filed of one or both data sets.\n"
                 << "* To disable this test, use the --disable-sanity-checks flag.\n"
                 << "* About to exit signaling failure\n"
                 << "*\n*\n*" );
      throw runtime_error( "Misaligned datasets" );
    }
  }

#define QF_DBG 0
#if QF_DBG
  quickfilter_box_type::debug_track_ids = make_pair( 5010, 158 );
#endif

  LOG_INFO( main_logger, "Adding quickfilter boxes to " << t.size() << " truth tracks..." );
  quickfilter_box_type::add_quickfilter_boxes( t, params );
  LOG_INFO( main_logger, "Adding quickfilter boxes to " << c.size() << " computed tracks..." );
  quickfilter_box_type::add_quickfilter_boxes( c, params );

  for ( unsigned i=0; i<t.size(); ++i )
  {
    if ((i % 10 == 0) || (i == t.size()-1)) {
      LOG_INFO( main_logger, "phase 1: " << i << " of " << t.size() << "..." );
    }
    for (unsigned j=0; j<c.size(); ++j)
    {
      this->compute_single( t[i], c[j] );
    }
  }

}

void
track2track_phase1
::compute_all_detection_mode( const track_handle_list_type& t,
                              const track_handle_list_type& c )
{
  track_field<track_oracle::dt::tracking::frame_number> fn;
  LOG_INFO( main_logger, "Phase 1 detection mode: aligning detections..." );

  typedef map< track_oracle::dt::tracking::frame_number::Type, pair< track_handle_list_type, track_handle_list_type > >::iterator i_t;
  map< track_oracle::dt::tracking::frame_number::Type, pair< track_handle_list_type, track_handle_list_type > > fn2gtct;
  for ( unsigned i=0; i<t.size(); ++i )
  {
    frame_handle_list_type f = track_oracle_core::get_frames( t[i] );
    if (f.size() != 1)
    {
      LOG_ERROR( main_logger, "Logic error: detection mode track had " << f.size() << " frames?" );
      return;
    }
    track_oracle::dt::tracking::frame_number::Type frame_number = fn( f[0].row );
    i_t probe = fn2gtct.find( frame_number );
    if (probe == fn2gtct.end())
    {
      pair< track_handle_list_type, track_handle_list_type > e;
      e.first.push_back( t[i] );
      fn2gtct[ frame_number ] = e;
    }
    else
    {
      probe->second.first.push_back( t[i] );
    }
  }

  LOG_INFO( main_logger, "Aligned truth; found " << fn2gtct.size() << " unique frame numbers" );

  for ( unsigned i=0; i<c.size(); ++i )
  {
    frame_handle_list_type f = track_oracle_core::get_frames( c[i] );
    if (f.size() != 1)
    {
      LOG_ERROR( main_logger, "Logic error: detection mode track had " << f.size() << " frames?" );
      return;
    }
    track_oracle::dt::tracking::frame_number::Type frame_number = fn( f[0].row );
    i_t probe = fn2gtct.find( frame_number );
    if (probe == fn2gtct.end())
    {
      pair< track_handle_list_type, track_handle_list_type > e;
      e.second.push_back( c[i] );
      fn2gtct[ frame_number ] = e;
    }
    else
    {
      probe->second.second.push_back( c[i] );
    }
  }
  LOG_INFO( main_logger, "Aligned truth and computed; found " << fn2gtct.size() << " unique frame numbers" );

  vul_timer timer;
  size_t counter=0;
  for (i_t i=fn2gtct.begin(); i != fn2gtct.end(); ++i)
  {
    if (timer.real() > 5 * 1000)
    {
      LOG_INFO( main_logger, "phase 1: " << counter << " of " << fn2gtct.size() << "..." );
      timer.mark();
    }
    ++counter;
    const track_handle_list_type& t_frame = i->second.first;
    const track_handle_list_type& c_frame = i->second.second;

    for (size_t ii=0; ii<t_frame.size(); ++ii)
    {
      for (size_t jj=0; jj<c_frame.size(); ++jj)
      {
        this->compute_single( t_frame[ii], c_frame[jj] );
      }
    }
  }
}

bool
track2track_phase1
::compute_single( track_handle_type t, track_handle_type c )
{
  track2track_type key = make_pair( t, c );

  // quick exit if we've already computed the score for this pair
  if ( this->t2t.find( key ) != this->t2t.end() )
  {
    return true;
  }

#define QF_DBG 0
#if QF_DBG
  quickfilter_box_type qf;
  bool use_radial_overlap = (params.radial_overlap >= 0.0);
  double qf_check = qf.quickfilter_check( t, c, use_radial_overlap );
#endif

  track2track_score t2t_score;
  bool b = t2t_score.compute( t, c, params );
  if ( b )
  {
    this->t2t[ key ] = t2t_score;
  }

#if QF_DBG
  if ((qf_check <= 0) && (! t2t_score.frame_overlaps.empty()))
  {
    static scorable_track_type local_track_view;
    unsigned t_id = local_track_view(t).external_id();
    unsigned c_id = local_track_view(c).external_id();
    LOG_ERROR( main_logger, "QF mismatch: qf check " << qf_check << " vs full " <<
               t2t_score.frame_overlaps.size() << " overlap frames : t " << t_id << " c " << c_id);
  }
#endif

  return b;
}

ts_type
track2track_phase1
::min_ts() const
{
  ts_type min = 0;

  typedef map< track2track_type, track2track_score >::const_iterator t2t_cit;
  for (t2t_cit i=this->t2t.begin(); i != this->t2t.end(); ++i)
  {
    if ( (i == this->t2t.begin()) || (i->second.overlap_frame_range.first < min))
    {
      min = i->second.overlap_frame_range.first;
    }
  }
  return min;
}

void
track2track_phase1
::debug_dump( const track_handle_list_type& gt_list,
              const track_handle_list_type& ct_list,
              const string& fn_prefix,
              ts_type ts_offset ) const
{
  // for ease of gnuplotting, output three files:
  // ${fn}-gt.dat  -- the ground truth tracks (green, say)
  // ${fn}-m0.dat  -- computed tracks which match some gt track (blue)
  // ${fn}-m1.dat  -- computed tracks which match NO ground truth track

  // The plotting routine will get the x-axis (timestamp) from the frame,
  // but we need to decide the y-axis (matched / not-matched) here.
  // The y-axis for ground-truth tracks will be assigned based on its order
  // in the for-loop; the y-axis for computed tracks will be assigned based
  // on the y-axis of the matched ground-truth frame (if any) or the y-axis
  // of the free-floating frames of the computed track.
  //
  // Thus, we'd like a quick way when iterating through the computed tracks
  // to know, on a frame-by-frame basis, what's the track handle for the
  // matching ground-truth tracks (if any).
  //
  // Sample gnuplot:
  //     plot "plot-gt.dat" ls 2 lw 2 w points t "ground truth", "plot-m0.dat" <backslash>
  //         ls 1 t "matched" w lines , "plot-m1.dat" w lines t "unmatched"
  //

  // this maps computed frames to the track handle of the matching ground-truth
  // track
  map< oracle_entry_handle_type, oracle_entry_handle_type > matched_computed_frame_map;

  typedef map< track2track_type, track2track_score >::const_iterator t2t_cit;
  for (t2t_cit i=this->t2t.begin(); i != this->t2t.end(); ++i)
  {
    const track_handle_type& gt_handle = i->first.first;
    for (unsigned j=0; j<i->second.frame_overlaps.size(); ++j)
    {
      const track2track_frame_overlap_record& r = i->second.frame_overlaps[j];
      matched_computed_frame_map[ r.computed_frame.row ] = gt_handle.row;
    }
  }

  // holds the y-axis assigned to a track handle
  map< oracle_entry_handle_type, unsigned int > y_axis_map;
  // remember which computed tracks have a dominant ground truth track
  map< oracle_entry_handle_type, bool > has_dominant_gt_track;

  unsigned int current_y_axis_index = 1;
  const unsigned int y_axis_spacing = 5;
  const unsigned int y_axis_offset = 2;
  scorable_track_type scorable_schema;

  // part 1: dump the ground truth tracks
  {
    string fn = fn_prefix + "-gt.dat";
    ofstream os( fn.c_str() );
    if ( ! os )
    {
      LOG_ERROR( main_logger, "Couldn't open '" << fn << "' for writing");
      return;
    }
    for (unsigned i=0; i<gt_list.size(); ++i)
    {
      unsigned int this_y_axis = (current_y_axis_index++) * y_axis_spacing;
      y_axis_map[ gt_list[i].row ] = this_y_axis;
      frame_handle_list_type frames = track_oracle_core::get_frames( gt_list[i] );

      for (unsigned j=0; j<frames.size(); ++j)
      {
        os << scorable_schema[ frames[j] ].timestamp_usecs() - ts_offset << " " << this_y_axis << "\n";
      }
      os << "\n\n";
    }
  }

  // part 2: decide which computed tracks have a dominant ground-truth track;
  // set the y-axis accordingly
  // track
  {
    for (unsigned i=0; i<ct_list.size(); ++i)
    {
      // key = ground-truth track, val = number of frames matched to that track
      map< oracle_entry_handle_type, unsigned > matched_track_census;
      for (unsigned j=0; j<gt_list.size(); ++j)
      {
        track2track_type key( gt_list[j], ct_list[i] );
        t2t_cit probe = this->t2t.find( key );
        if (probe != this->t2t.end())
        {
          matched_track_census[ gt_list[j].row ] += probe->second.frame_overlaps.size();
        }
      }

      // Naively, one would expect that at most one key to the census
      // will have more than half the total frames.  However, this
      // seems not to be the case.  Given the size of the bounding
      // boxes in the ground truth, maybe this isn't surprising.  So instead
      // assign the dominance to whoever matches the most frames (but is over
      // half.)

      frame_handle_list_type frames = track_oracle_core::get_frames( ct_list[i] );
      vector< oracle_entry_handle_type > dominant_tracks;
      for (map<oracle_entry_handle_type, unsigned>::const_iterator
             j = matched_track_census.begin();
           j != matched_track_census.end();
           ++j)
      {
        if (j->second > (frames.size()/2) )
        {
          if (dominant_tracks.empty() )
          {
            dominant_tracks.push_back( j->first );
          }
          else
          {
            dominant_tracks[0] =
              (dominant_tracks[0] > j->second)
              ? dominant_tracks[0]
              : j->second;
          }
        }
      }

      if ( dominant_tracks.size() > 1)
      {
        // if we ever see this, we'll add more informative output...
        LOG_ERROR( main_logger, "Major logic error: computed track with multiple dominant tracks?");
        return;
      }

      unsigned int this_y_axis =
        dominant_tracks.empty()
        ? (current_y_axis_index++) * y_axis_spacing // give it its own slot
        : y_axis_map[ dominant_tracks[0] ] - y_axis_offset; // offset from dominant track

      y_axis_map[ ct_list[i].row ] = this_y_axis;
      has_dominant_gt_track[ ct_list[i].row ] = ( ! dominant_tracks.empty() );

    } // ... for each computed track
  }

  // part 3: write out the computed tracks to -m0.dat or -m1.dat,
  // depending on if they have a dominant track or not
  {
    string fn_m0 = fn_prefix + "-m0.dat";
    string fn_m1 = fn_prefix + "-m1.dat";
    ofstream os_m0( fn_m0.c_str() );
    if (! os_m0)
    {
      LOG_ERROR( main_logger, "Couldn't open '" << fn_m0 << "' for writing");
      return;
    }
    ofstream os_m1( fn_m1.c_str() );
    if (! os_m1)
    {
      LOG_ERROR( main_logger, "Couldn't open '" << fn_m1 << "' for writing");
      return;
    }

    for (unsigned i=0; i<ct_list.size(); ++i)
    {
      frame_handle_list_type frames = track_oracle_core::get_frames( ct_list[i] );
      unsigned default_y_axis = y_axis_map[ ct_list[i].row ];
      ofstream& os =
        has_dominant_gt_track[ ct_list[i].row ]
        ? os_m0
        : os_m1;

      map<oracle_entry_handle_type, oracle_entry_handle_type>::const_iterator probe;
      for (unsigned j=0; j<frames.size(); ++j)
      {
        probe = matched_computed_frame_map.find( frames[j].row );
        unsigned this_y_axis =
          (probe == matched_computed_frame_map.end())
          ? default_y_axis
          : y_axis_map[ probe->second ];
        ts_type ts = scorable_schema[ frames[j] ].timestamp_usecs();
        if ( ts != 0)
        {
          os << scorable_schema[ frames[j] ].timestamp_usecs() - ts_offset << " " << this_y_axis << "\n";
        }
        else
        {
          LOG_INFO( main_logger, "Frame w/ 0 ts? " << scorable_schema[frames[j]]<<"");
        }
      }
      os << "\n\n";
    }
  }

  // all done!
}

vector< track2track_type >
track2track_phase1
::matching_keys( const track2track_type& probe ) const
{
  // exactly one of probe must be undefined.  The 'search' key is the other.
  // Return a list of all t2t keys with the search key in that position.
  bool first_is_undef = probe.second.is_valid();
  track_handle_type k = (first_is_undef) ? probe.second : probe.first;
  vector< track2track_type > ret;
  for (map< track2track_type, track2track_score >::const_iterator i = this->t2t.begin();
       i != this->t2t.end();
       ++i)
  {
    bool hit =
      first_is_undef
      ? i->first.second == k
      : i->first.first == k;
    if ( hit )
    {
      ret.push_back( i->first );
    }
  }
  return ret;
}

void
track2track_phase1
::debug_dump( ostream & os )
{
  scorable_track_type scorable_track;
  typedef map< track2track_type, track2track_score >::const_iterator it;

  os << "##Header####GT id : CP id : ([Frame Time GT]; [Frame Time CP]; Area_overlap; Percentage GT Overlap; Percentage CP Overlap),...." << endl;
  for ( it i = this->t2t.begin(); i != this->t2t.end(); ++i )
  {
    os << "GT " << scorable_track( i->first.first ).external_id() << " : CP " << scorable_track( i->first.second ).external_id();
    vector< track2track_frame_overlap_record > const & frame_overlap = i->second.frame_overlaps;
    for( unsigned int j = 0; j < frame_overlap.size(); ++j )
    {
      track2track_frame_overlap_record const & t2tfo = frame_overlap[j];
      os << " ([" << scorable_track[t2tfo.truth_frame].timestamp_frame() << " " << scorable_track[t2tfo.truth_frame].timestamp_usecs() << "]; ["
         << scorable_track[t2tfo.computed_frame].timestamp_frame() << " " << scorable_track[t2tfo.computed_frame].timestamp_usecs() << "]; "
         << t2tfo.overlap_area << ";" << t2tfo.overlap_area/t2tfo.truth_area << ";" <<  t2tfo.overlap_area/t2tfo.computed_area << ")";
    }
    os << endl;
  } // all gt tracks
}

descriptor_overlap_type
track2track_score
::create_overlap_descriptor() const
{
  descriptor_overlap_type ret;

  // log a warning and return if cached handles not set (i.e. compute has not been called)
  if ( ! ( this->cached_truth_track.is_valid() && this->cached_comp_track.is_valid()))
  {
    LOG_WARN( main_logger, "Attempted to create overlap descriptor before calling compute()" );
    return ret;
  }

  track_field<unsigned> external_id("external_id");
  track_field<int> activity_id ("activity" );
  if (external_id.exists( this->cached_truth_track.row ))
  {
    ret.src_trk_id = external_id( this->cached_truth_track.row );
  }
  if (external_id.exists( this->cached_comp_track.row ))
  {
    ret.dst_trk_id = external_id( this->cached_comp_track.row );
  }
  ret.src_activity_id =
    activity_id.exists( this->cached_truth_track.row )
    ? activity_id( this->cached_truth_track.row )
    : aries_interface::activity_to_index( "NotScored" );
  ret.dst_activity_id =
    activity_id.exists( this->cached_comp_track.row )
    ? activity_id( this->cached_comp_track.row )
    : aries_interface::activity_to_index( "NotScored" );

  ret.n_frames_src = track_oracle_core::get_n_frames( this->cached_truth_track );
  ret.n_frames_dst = track_oracle_core::get_n_frames( this->cached_comp_track );
  ret.n_frames_overlap = this->frame_overlaps.size();
  if (ret.n_frames_overlap == 0)
  {
    LOG_WARN( main_logger, "Creating overlap descriptor with zero overlap frames?" );
    return ret;
  }

  bool overall_radial_flag = false;
  double sum_centroid_distance = 0.0;
  double sum_percentage_overlap = 0.0;
  for (size_t i=0; i<this->frame_overlaps.size(); ++i)
  {
    const track2track_frame_overlap_record& r = this->frame_overlaps[i];
    if (i == 0)
    {
      overall_radial_flag = r.truth_area < 0;
    }
    bool this_radial_flag = r.truth_area < 0;
    if ( this_radial_flag != overall_radial_flag)
    {
      throw runtime_error( "Inconsistent radial flag setting descriptor!" );
    }

    sum_centroid_distance += r.centroid_distance;
    if ( ! overall_radial_flag )
    {
      double this_union_area = r.truth_area + r.computed_area - r.overlap_area;
      double this_percentage_overlap =
        ( this_union_area == 0.0 )
        ? 0.0
        : r.overlap_area / this_union_area;
      sum_percentage_overlap += this_percentage_overlap;
    }
  }
  ret.radial_overlap_flag = overall_radial_flag;
  ret.mean_centroid_distance = sum_centroid_distance / ret.n_frames_overlap;
  if ( ! overall_radial_flag )
  {
    ret.mean_percentage_overlap = sum_percentage_overlap / ret.n_frames_overlap;
  }

  return ret;
}

void
track2track_score
::add_self_to_event_label_descriptor( descriptor_event_label_type& delt ) const
{
  // log a warning and return if cached handles not set (i.e. compute has not been called)
  if ( ! ( this->cached_truth_track.is_valid() && this->cached_comp_track.is_valid()))
  {
    LOG_WARN( main_logger, "Attempted to create overlap descriptor before calling compute()" );
    return;
  }

  single_event_label_type s;

  // assume activity is coming from the source track
  track_field<int> activity_id ("activity" );
  s.activity_name = "NotScored";
  if ( activity_id.exists(this->cached_truth_track.row ) )
  {
    const map< size_t, string >& i2a = aries_interface::index_to_activity_map();
    typedef map< size_t, string >::const_iterator i2a_cit;
    i2a_cit probe = i2a.find( activity_id( this->cached_truth_track.row ) );
    if (probe != i2a.end() )
    {
      s.activity_name = probe->second;
    }
  }


  unsigned n_frames_dst = track_oracle_core::get_n_frames( this->cached_comp_track );
  unsigned n_frames_overlap = this->frame_overlaps.size();
  if (n_frames_overlap == 0)
  {
    LOG_WARN( main_logger, "Creating overlap descriptor with zero overlap frames?" );
    return;
  }

  bool overall_radial_flag = false;
  double sum_centroid_distance = 0.0;
  double sum_percentage_overlap = 0.0;
  for (size_t i=0; i<this->frame_overlaps.size(); ++i)
  {
    const track2track_frame_overlap_record& r = this->frame_overlaps[i];
    if (i == 0)
    {
      overall_radial_flag = r.truth_area < 0;
    }
    bool this_radial_flag = r.truth_area < 0;
    if ( this_radial_flag != overall_radial_flag)
    {
      throw runtime_error( "Inconsistent radial flag setting descriptor!" );
    }

    sum_centroid_distance += r.centroid_distance;
    if ( ! overall_radial_flag )
    {
      double this_union_area = r.truth_area + r.computed_area - r.overlap_area;
      double this_percentage_overlap =
        ( this_union_area == 0.0 )
        ? 0.0
        : r.overlap_area / this_union_area;
      sum_percentage_overlap += this_percentage_overlap;
    }
  }


  // temporal overlap: what percentage of the COMPUTED interval was overlap?
  s.temporal_overlap = 1.0 * n_frames_overlap / n_frames_dst;

  // spatial overlap: for radial, use a sigmoid such that:
  // - when distance is 0, overlap = 1
  // - when distance is params.radial_overlap, overlap = 0

  if ( overall_radial_flag )
  {
    LOG_WARN ( main_logger,"Haven't implemented radial 'spatial overlap' yet...");
    return;
  }
  else
  {
    s.spatial_overlap = sum_percentage_overlap / n_frames_overlap;
  }

  delt.labels.push_back( s );
}

} // ...kwant
} // ...kwiver

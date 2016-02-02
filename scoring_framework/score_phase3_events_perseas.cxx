/*ckwg +5
 * Copyright 2010-2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "score_phase3_events_perseas.h"

#include <vgl/vgl_intersection.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <track_oracle/scoring_framework/score_phase2_hadwav.h>
#include <vul/vul_awk.h>
#include <boost/lexical_cast.hpp>

#include <event_detectors/event_reader.h>

#include <logger/logger.h>


using std::getline;
using std::ifstream;
using std::istringstream;
using std::make_pair;
using std::map;
using std::multimap;
using std::ostream;
using std::pair;
using std::string;
using std::vector;


#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __vidtk_logger_auto_score_phase3_events_perseas_cxx__
VIDTK_LOGGER("score_phase3_events_perseas_cxx");


unsigned int vidtk::track_event_label::event_id_counter = 0;

namespace // anon
{

unsigned int
id_of( vidtk::track_handle_type id )
{
  if (id.row == vidtk::INVALID_ROW_HANDLE) return static_cast<unsigned int>( -1 );

  static vidtk::scorable_track_type t;
  return t( id ).external_id();
}

} // anon namespace

namespace vidtk
{

bool
phase3_events
::load_track_events_map( const string& filename,
                         vector< event_id_type >& event_list,
                         domain_handle_type domain,
                         const vgl_box_2d<double>& aoi,
                         double point_window_in_frames )
{
  scorable_track_type t;

  ifstream is( filename.c_str() );
  if ( ! is )
  {
    LOG_ERROR( "Couldn't load events map '" << filename << "'");
    return false;
  }

  unsigned events_read = 0;
  unsigned events_kept = 0;

  string tmp;
  while ( getline(is, tmp) )
  {
    // format is label track_id start_frame end_frame prob
    istringstream iss( tmp );

    double t0, t1, prob;
    unsigned id;
    string label;

    if ( ! ( iss >> label >> id >> t0 >> t1 >> prob ))
    {
      LOG_ERROR( "Couldn't parse '" << tmp << "'");
      return false;
    }
    ++events_read;

    if ((domain == this->gt_domain) && (t0 == t1))
    {
      // adjust timestamps for groundtruth points
      t0 -= point_window_in_frames;
      t1 += point_window_in_frames;  // 5.0 == 2.5secs
    }

    // look up the id
    oracle_entry_handle_type row = t.external_id.lookup( id, domain );
    if ( row == INVALID_ROW_HANDLE )
    {
      LOG_INFO( "Events map '" << filename << "': track " << id << " not loaded");
      continue;
    }

    // insert an entry for the track
    track_event_label event( track_handle_type(row), id, label, t0, t1, prob );
    if ( this->intersect_event_with_aoi( event, aoi ))
    {
      this->event_map.insert( make_pair( event.event_id, event ));
      this->track_to_event_map.insert( make_pair( track_handle_type(row), event.event_id ) );
      event_list.push_back( event.event_id );
      ++events_kept;
    }
  }

  // verify that all tracks in the domain now have a label
  track_handle_list_type tracks =
    vidtk::track_oracle::generic_to_track_handle_list( vidtk::track_oracle::get_domain( domain ) );
  unsigned good_count = 0, bad_count = 0;
  for (unsigned i=0; i<tracks.size(); ++i)
  {
    if ( this->track_to_event_map.find( tracks[i] ) != this->track_to_event_map.end() )
    {
      ++good_count;
    }
    else
    {
      ++bad_count;
      track_event_label event( tracks[i], id_of( tracks[i] ), "DEFAULT", -1.0, -1.0, -1.0 );
      if ( this->intersect_event_with_aoi( event, aoi ))
      {
        this->event_map.insert( make_pair( event.event_id, event ));
        this->track_to_event_map.insert( make_pair( tracks[i], event.event_id ));
        event_list.push_back( event.event_id );
      }
    }
  }
  LOG_INFO( good_count << " tracks with labels; " << bad_count << " set to default");
  LOG_INFO( "AOI kept " << events_kept << " events of " << events_read << " read from file");

  return true;
}


bool
phase3_events
::load_gt_events_map( const string& filename,
                      const track_handle_list_type& handles,
                      const vgl_box_2d<double>& aoi,
                      double point_window_in_frames )
{
  this->gt_domain = track_oracle::create_domain( track_oracle::track_to_generic_handle_list( handles ) );
  return this->load_track_events_map( filename, this->gt_event_list, this->gt_domain, aoi, point_window_in_frames );
}

bool
phase3_events
::load_computed_events_map( const string& filename, const track_handle_list_type& handles, const vgl_box_2d<double>& aoi )
{
  this->computed_domain = track_oracle::create_domain( track_oracle::track_to_generic_handle_list( handles ));
  return this->load_track_events_map( filename, this->computed_event_list, this->computed_domain, aoi, -1.0 );
}

bool
phase3_events
::intersect_event_with_aoi( const track_event_label& event, const vgl_box_2d<double>& aoi ) const
{
  if ( aoi.is_empty() )
  {
    return true;
  }

  scorable_track_type t;
  vidtk::field_handle_type bbox_field = t.bounding_box.get_field_handle();

  bool any_frames_intersect = false;
  unsigned checked(0), passed(0);

  frame_handle_list_type frames = track_oracle::get_frames( event.internal_id );
  for (unsigned i = 0; i < frames.size(); ++i)
  {
    double frame_ts = static_cast<double>( t[ frames[i] ].timestamp_usecs() );
    // need to convert "frame TS" into a frame number
    frame_ts /= (1000 * 1000);
    frame_ts *= 2;
    //    LOG_INFO( " ..." << event.start_time << ": " << event.end_time << " :: " << frame_ts << "; bbox: " << track_oracle::field_has_row<vgl_box_2d<double> >( frames[i].row, bbox_field ) << "");
    if ( ( event.start_time <= frame_ts )
         && ( frame_ts <= event.end_time )
         && ( track_oracle::field_has_row( frames[i].row, bbox_field )) )
    {
      ++checked;
      vgl_box_2d<double> frame_box = t[ frames[i] ].bounding_box();
      vgl_box_2d<double> overlap = vgl_intersection( frame_box, aoi );
      if ( ! overlap.is_empty() )
      {
        any_frames_intersect = true;
        ++passed;
      }
    }
  }

  //  LOG_INFO( event.label << " on " << event.external_id << ": " << checked << " checked; " << passed << " passed ( " << frames.size() << " frames)");
  return any_frames_intersect;
}



vector< event_id_type >
phase3_events
::find_overlapping_events( const track2track_phase2_hadwav& p2,
                           const track_event_label& src_event,
                           bool src_is_gt,
                           int& n_track_matches )
{
  typedef map< track_handle_type, track_handle_list_type >::const_iterator p2it;
  typedef multimap< track_handle_type, event_id_type >::const_iterator p3it;

  vector< unsigned int > matching_event_ids;

  const track_handle_type src_track = src_event.internal_id;

  // First, use either the t2c or c2t maps to get the list of
  // opposing tracks which had ANY overlap; this list will be
  // winnowed down based on the event's time window
  const map< track_handle_type, track_handle_list_type >& p2map =
    (src_is_gt)
    ? p2.t2c   // use the truth-to-computed list
    : p2.c2t;  // use the computed-to-truth list

  p2it p2_probe = p2map.find( src_track );
  // early exit if the source track didn't match any opposing tracks
  // at the most basic level
  if ( p2_probe == p2map.end() )
  {
    n_track_matches = 0;
    return matching_event_ids;
  }

  // Each track in p2map may have multiple events in the p3map; hence
  // the two-level loop: once for each track T that matches, another
  // for each event associated with T.

  const track_handle_list_type& opposing_track_list = p2_probe->second;
  n_track_matches = opposing_track_list.size();
  for (unsigned i=0; i<opposing_track_list.size(); ++i)
  {
    pair< p3it, p3it > opposing_events = this->track_to_event_map.equal_range( opposing_track_list[i] );
    for (p3it j = opposing_events.first; j != opposing_events.second; ++j)
    {
      const track_event_label& opposing_event = this->event_map[ j->second ];

      // do the timestamps overlap?
      bool out_of_bounds =
        (src_event.end_time < opposing_event.start_time) ||
        (src_event.start_time > opposing_event.end_time);

      if ( ! out_of_bounds )
      {
        matching_event_ids.push_back( j->second );
      }
    }
  }

  return matching_event_ids;
}

void
phase3_events
::compute( const track2track_phase2_hadwav& p2, double roc_point, bool first_pass, ostream* log_os )

{
  scorable_track_type t;

  // Record what computed events are matched to ground truth events
  // and thus removed from false-positive consideration.
  map< event_id_type, bool > match_map;

  //
  // First, go through all the ground truth events.
  //

  for (unsigned gt_index = 0; gt_index < this->gt_event_list.size(); ++gt_index)
  {
    const track_event_label& this_gt_event = this->event_map[ this->gt_event_list[ gt_index ]];
    event_count& event_stats = this->event_stats_map[ this_gt_event.label ];

    // What events overlap the this event in time and space, ignoring the ROC point for now?
    int n_track_matches = 0;
    vector< event_id_type > overlapping_computed_event_list
      = this->find_overlapping_events( p2, this_gt_event, /* is gt = */ true, n_track_matches );

    // Any event whose label matches and is at or above the ROC point is a match.
    // Multiple matches count as a single hit; all matched computed events are
    // eliminated from the false-positive sweep (as in VIRAT.)

    bool any_matches = false;
    if ( log_os && first_pass )
    {
      (*log_os) << "gt event " << this_gt_event.event_id << " (" << this_gt_event.label << ") "
                << "track " << this_gt_event.external_id << " @ " << this_gt_event.start_time << ":"
                << this_gt_event.end_time << " overlaps "
                << overlapping_computed_event_list.size() << " computed events; "
                << n_track_matches <<  " track matches\n";
    }
    for (unsigned i=0; i<overlapping_computed_event_list.size(); ++i)
    {
      event_id_type other_event_id = overlapping_computed_event_list[i];
      const track_event_label& other_event = this->event_map[ other_event_id ];

      // for plotting purposes, report the track-level event matches on the first pass,
      // before looking at the roc point
      if ( log_os && first_pass )
      {
        (*log_os) << "  " << i << ": "
                  << "computed event "
                  << other_event.external_id << " (" << other_event.label << " [p " << other_event.prob << "]) @ "
                  << other_event.start_time << ":" << other_event.end_time << "\n";
      }

      bool matches_at_roc_point
        = ( this_gt_event.label == other_event.label )
        && ( other_event.prob >= roc_point );
      if (matches_at_roc_point)
      {
        any_matches = true;
        match_map[ other_event_id ] = true;
      }
    }

    if (any_matches)
    {
      ++event_stats.tp_count;
    }
    else
    {
      ++event_stats.fn_count;
    }
  } // ...for all gt events

  //
  // next, sweep through the computed events looking for un-matched events.
  // If the event is above the ROC point, then it's a false positive.
  // If the event is below the ROC point, it's a true negative.  (Double-check this;
  // true negatives were a sticking point in VIRAT, but we don't yet have a
  // HADWAV normalization factor to generate FARs in place of a PFA.)
  //

  for (unsigned i=0; i<this->computed_event_list.size(); ++i)
  {
    event_id_type computed_event_id = this->computed_event_list[ i ];

    // skip events already matched to a ground-truth event
    if (match_map[ computed_event_id ])
    {
      continue;
    }

    const track_event_label& computed_event = this->event_map[ computed_event_id ];
    event_count& event_stats = this->event_stats_map[ computed_event.label ];
    if ( log_os && first_pass )
    {
      (*log_os) << "match comp " << computed_event.external_id << " " << computed_event.label << " " << computed_event.prob << "\n";
    }

    if (computed_event.prob >=  roc_point)
    {
      ++event_stats.fp_count;
    }
    else
    {
      ++event_stats.tn_count;
    }
  } // ...for all computed events
}

void
phase3_events
::clear_stats( void )
{
  for ( map< string, event_count >::iterator i = this->event_stats_map.begin();
        i != this->event_stats_map.end();
        ++i )
  {
    i->second = event_count();
  }
}

} // namespace vidtk

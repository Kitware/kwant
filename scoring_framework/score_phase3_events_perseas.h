/*ckwg +5
 * Copyright 2010 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_SCORE_PHASE3_EVENTS_PERSEAS_H
#define INCL_SCORE_PHASE3_EVENTS_PERSEAS_H

// given phase-2 results (gt tracks have been associated with computed
// tracks), load in a list of (track_id, event) pairs for both sets and
// compute basic pD/FAR for events.

#include <string>
#include <map>
#include <iostream>
#include <track_oracle/core/track_oracle.h>
#include <scoring_framework/score_phase2_hadwav.h>

#include <vgl/vgl_box_2d.h>

namespace vidtk
{

class track2track_phase2;

typedef unsigned int event_id_type;

struct track_event_label
{
  event_id_type event_id;
  track_handle_type internal_id;
  unsigned int external_id;
  std::string label;
  double start_time;
  double end_time;
  double prob;
  track_event_label()
    : event_id( ++track_event_label::event_id_counter ),
      internal_id( track_handle_type() ),
      external_id( static_cast<unsigned int>( -1 ) ),
      label( "no-label-set" ),
      start_time( -1.0 ),
      end_time( -1.0 ),
      prob( -1.0 )
  {}
  track_event_label( track_handle_type ext_id, unsigned int id, const std::string& lab, double t0, double t1, double p )
    : event_id( ++track_event_label::event_id_counter ),
      internal_id( ext_id ),
      external_id( id ),
      label( lab ),
      start_time( t0 ),
      end_time( t1 ),
      prob( p )
  {}

private:
  static event_id_type event_id_counter;
};

struct event_count
{
  unsigned tp_count; // true positives
  unsigned fp_count; // false positives
  unsigned tn_count; // true negatives (actually, hard to calculate)
  unsigned fn_count; // false negatives
  event_count()
    : tp_count(0), fp_count(0), tn_count(0), fn_count(0)
  {}
};

struct phase3_events
{
  // Holds every individual event, both computed and ground-truth
  std::map< event_id_type, track_event_label > event_map;

  // Maps each event label to its summary statistics structure
  std::map< std::string, event_count > event_stats_map;

  // enumerates the event IDs of the computed ground-truth and events
  std::vector< event_id_type > gt_event_list;
  std::vector< event_id_type > computed_event_list;


  void compute( const track2track_phase2_hadwav& p2, double roc_point, bool first_pass, std::ostream* log_os = 0 );
  void clear_stats( void );
  bool load_gt_events_map( const std::string& filename,
                           const track_handle_list_type& handles,
                           const vgl_box_2d<double>& aoi,
                           double point_window_in_frames );
  bool load_computed_events_map( const std::string& filename,
                                 const track_handle_list_type& handles,
                                 const vgl_box_2d<double>& aoi );

private:
  // cache for quicker lookup
  std::multimap< track_handle_type, event_id_type > track_to_event_map;

  bool intersect_event_with_aoi( const track_event_label& event,
                                 const vgl_box_2d<double>& aoi ) const;

  bool load_track_events_map( const std::string& filename,
                              std::vector< event_id_type >& event_list,
                              domain_handle_type domain,
                              const vgl_box_2d<double>& aoi,
                              double point_window_in_frames );
  std::vector< event_id_type > find_overlapping_events( const track2track_phase2_hadwav& p2,
                                                       const track_event_label& src_event,
                                                       bool src_is_gt,
                                                       int& n_track_matches );

  domain_handle_type gt_domain, computed_domain;

};

} // namespace vidtk

#endif

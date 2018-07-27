/*ckwg +5
 * Copyright 2010-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "score_tracks_hadwav.h"
#include <cstdlib>

#include <track_oracle/core/state_flags.h>
#include <stdexcept>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );


using std::exit;
using std::make_pair;
using std::map;
using std::ostream;
using std::ostringstream;
using std::pair;
using std::runtime_error;
using std::string;

using kwiver::track_oracle::frame_handle_list_type;
using kwiver::track_oracle::frame_handle_type;
using kwiver::track_oracle::track_field;
using kwiver::track_oracle::track_handle_list_type;
using kwiver::track_oracle::track_handle_type;
using kwiver::track_oracle::track_oracle_core;
using kwiver::kwant::track2track_type;
using kwiver::kwant::track2track_scalars_hadwav;
using kwiver::kwant::scorable_track_type;

typedef map< track_handle_type, track_handle_list_type >::const_iterator t2t_it;


namespace { // anon

unsigned
find_dominant( const map< track2track_type, track2track_scalars_hadwav >& t2t,
               track_handle_type source,
               const track_handle_list_type& tracks,
               bool find_computed_dominated_by_target )
{
  // this searches down the value of an (c2t, t2c) entry, looking for
  // the dominant match.  If the value is non-empty, there should be
  // exactly one dominant match.
  unsigned match_index = static_cast<unsigned>( -1 );
  for ( unsigned i=0; i<tracks.size(); ++i )
  {
    // the fact that this idiom keeps appearing in the code is a clue
    // that the design is... non-optimal. :)
    track2track_type key =
      (find_computed_dominated_by_target)
      ? make_pair( tracks[i], source )   // source is computed
      : make_pair( source, tracks[i] );  // source is ground truth

    map< track2track_type, track2track_scalars_hadwav >::const_iterator probe = t2t.find( key );
    if ( probe == t2t.end() )
    {
      ostringstream oss;
      oss << "Logic bomb: lost t2t entry for " << key.first.row << ", " << key.second.row << " sense " << find_computed_dominated_by_target << "\n";
      for (map< track2track_type, track2track_scalars_hadwav >::const_iterator j = t2t.begin();
           j != t2t.end();
           ++j )
      {
        oss << "key : " << j->first.first.row << ", " << j->first.second.row << "\n";
      }
      throw runtime_error( oss.str() );
    }

    bool is_match =
      (find_computed_dominated_by_target)
      ? probe->second.computed_is_dominated_by_target
      : probe->second.target_is_dominated_by_computed;

    if (is_match)
    {
      if ( match_index != static_cast<unsigned>( -1 ) )
      {
        ostringstream oss;
        oss << "Logic bomb: source " << source.row << " has multiple dominators; key is " << key.first.row << ", " << key.second.row << "\n";
        throw runtime_error( oss.str() );
      }
      match_index = i;
    }
  }

  return match_index;
}

void
debug_dump_output_key( ostream& os,
                       const track_handle_type& t )
{
  static scorable_track_type scorable_track;
  frame_handle_list_type frames = track_oracle_core::get_frames( t );
  unsigned first_frame_num = scorable_track[ frames[0] ].timestamp_frame();
  unsigned track_id = scorable_track( t ).external_id();
  os << track_id << " " << frames.size() << " " << first_frame_num;
}

void
debug_dump_entry( ostream& os,
                 const map< track2track_type, track2track_scalars_hadwav >& t2t,
                 const string& tag,
                 t2t_it src,
                 bool src_is_computed )
{
  const auto& matches = src->second;
  os << tag
     << " : ";
  debug_dump_output_key( os, src->first );
  os << " : "
     << matches.size()
     << " : ";

  if ( ! matches.empty() )
  {
    //
    // Always output the "dominant" match first
    //

    auto dominant_index = find_dominant( t2t, src->first, src->second, src_is_computed );
    debug_dump_output_key( os, matches[ dominant_index ] );
    for (unsigned j=0; j<matches.size(); ++j)
    {
      if (j == dominant_index) continue;
      os << " ; ";
      debug_dump_output_key( os, matches[j] );
    }
  }
}

} // ...anon

namespace kwiver {
namespace kwant {


void
track2track_phase2_hadwav
::compute( const track_handle_list_type& t,
           const track_handle_list_type& c,
           const track2track_phase1& p1 )
{
  // MITRE's "target" == ground truth
  // MITRE's "track" == computed track

  static scorable_track_type scorable_track;

  this->n_true_tracks = t.size();
  this->n_computed_tracks = c.size();

  if (this->verbose)
  {
    LOG_INFO( main_logger, "phase2: " << this->n_true_tracks << " true tracks");
    LOG_INFO( main_logger, "phase2: " << this->n_computed_tracks << " computed tracks");
  }
  // insert empty scalars
  for (map< track2track_type, track2track_score>::const_iterator i = p1.t2t.begin();
       i != p1.t2t.end();
       ++i )
  {
    this->t2t[ i->first ] = track2track_scalars_hadwav();
  }
  for (unsigned i=0; i<n_true_tracks; ++i)
  {
    this->t2c[ t[i] ] = track_handle_list_type();
  }
  for (unsigned i=0; i<n_computed_tracks; ++i)
  {
    this->c2t[ c[i] ] = track_handle_list_type();
  }

  // for each computed track: is it associated with any ground truth track?
  // raw material for MITRE's track continuity

  size_t total_gt_boxes = 0;
  size_t total_computed_boxes = 0;

  size_t detected_gt_boxes = 0;

  map<pair<track_handle_type, frame_handle_type>, bool> ct_frame_marker;

  for (size_t i = 0; i < c.size(); ++i)
  {
    track_handle_type const& ct = c[i];
    frame_handle_list_type const frames = track_oracle_core::get_frames( ct );

    total_computed_boxes += scorable_track( ct ).frames_in_aoi();

    for (size_t f = 0; f < frames.size(); ++f)
    {
      frame_handle_type const& frame = frames[f];

      ct_frame_marker[make_pair(ct, frame)] = false;
    }
  }

  this->detectionFalseAlarms = total_computed_boxes;

  for (size_t g = 0; g < t.size(); ++g)
  {
    track_handle_type const& gt = t[g];
    frame_handle_list_type const frames = track_oracle_core::get_frames( gt );

    total_gt_boxes += scorable_track( gt ).frames_in_aoi();

    map<frame_handle_type, bool> gt_frame_marker;

    for (size_t f = 0; f < frames.size(); ++f)
    {
      frame_handle_type const& frame = frames[f];
      unsigned match_state = scorable_track[ frame ].frame_has_been_matched();
      if ( ( match_state == IN_AOI_UNMATCHED ) ||
           ( match_state == IN_AOI_MATCHED ))
      {
        gt_frame_marker[frame] = false;
      }
    }

    for (size_t i = 0; i < c.size(); ++i)
    {
      track_handle_type const& ct = c[i];
      track2track_type const key = make_pair(gt, ct);
      map<track2track_type, track2track_score>::const_iterator const probe = p1.t2t.find(key);

      if (probe == p1.t2t.end())
      {
        continue;
      }

      track2track_score const& score = probe->second;

      for (size_t f = 0; f < score.frame_overlaps.size(); ++f)
      {
        track2track_frame_overlap_record const& overlap = score.frame_overlaps[f];
        unsigned match_state = scorable_track[ overlap.computed_frame ].frame_has_been_matched();
        bool cp_is_in_aoi =  ( match_state == IN_AOI_UNMATCHED ) || ( match_state == IN_AOI_MATCHED );
        match_state = scorable_track[ overlap.truth_frame ].frame_has_been_matched();
        bool gt_is_in_aoi = ( match_state == IN_AOI_UNMATCHED ) || ( match_state == IN_AOI_MATCHED );
        if(!(cp_is_in_aoi && gt_is_in_aoi))
        {
          continue;
        }

        if (!gt_frame_marker[overlap.truth_frame])
        {
          ++detected_gt_boxes;
          gt_frame_marker[overlap.truth_frame] = true;
        }

        pair<track_handle_type, frame_handle_type> const track_key = make_pair(ct, overlap.computed_frame);

        if (!ct_frame_marker[track_key])
        {
          --this->detectionFalseAlarms;
          ct_frame_marker[track_key] = true;
        }
      }
    }
  }

  for (map< track2track_type, track2track_scalars_hadwav>::iterator i = this->t2t.begin();
       i != this->t2t.end();
       ++i )
  {
    track2track_type key = i->first;
    track_handle_type t_id = i->first.first;
    track_handle_type c_id = i->first.second;
    map< track2track_type, track2track_score >::const_iterator p1_scores = p1.t2t.find( key );
    if ( p1_scores == p1.t2t.end() )
    {
      continue;
    }
    const track2track_score& s = p1_scores->second;
    // it is associated if it has non-zero spatio-temporal overlap
    if ( s.spatial_overlap_total_frames > 0 )
    {
      i->second.computed_associated_with_target = true;
      i->second.computed_frames_on_target = s.spatial_overlap_total_frames;

      // record association
      this->c2t[ c_id ].push_back( t_id );
      this->t2c[ t_id ].push_back( c_id );
      if (this->verbose)
      {
        unsigned eid1 = scorable_track(i->first.first).external_id();
        size_t nf1 = track_oracle_core::get_n_frames( i->first.first );
        unsigned eid2 = scorable_track(i->first.second).external_id();
        size_t nf2 = track_oracle_core::get_n_frames( i->first.second );
        LOG_INFO( main_logger, "phase2:  true-to-computed association " << eid1
                 << "[ len " << nf1 << " ] "
                 << eid2
                 << "[ len " << nf2 << " ] "
                 << ": associated; overlap "
                 << s.spatial_overlap_total_frames << " frames");
      }
    }
  } // ...for all true/computed pairs

  // compute dominators for targets
  for ( map< track_handle_type, track_handle_list_type >::const_iterator i = this->t2c.begin();
        i != this->t2c.end();
        ++i )
  {
    track_handle_type max_computed_track;
    unsigned max_frames = 0;
    const track_handle_list_type& tracks_on_target = i->second;
    if ( ! tracks_on_target.empty() )
    {
      for (unsigned j=0; j<tracks_on_target.size(); ++j)
      {
        track2track_type key = make_pair( i->first, tracks_on_target[j] );
        map< track2track_type, track2track_scalars_hadwav >::const_iterator probe = this->t2t.find( key );
        if ( probe == this->t2t.end() )
        {
          LOG_ERROR( main_logger, "Logic error: lost c2t pair?");
          exit(1);
        }
        const track2track_scalars_hadwav& s = probe->second;
        if ( s.computed_frames_on_target == 0 )
        {
          LOG_ERROR( main_logger, "Logic error: c2t contains 0 computed frames on target?");
          exit(1);
        }

        if ( s.computed_frames_on_target > max_frames )
        {
          max_frames = s.computed_frames_on_target;
          max_computed_track = tracks_on_target[j];
        }
      }
      if ( max_computed_track.row == kwiver::track_oracle::INVALID_ROW_HANDLE )
      {
        LOG_ERROR( main_logger, "Logic error: c2t entry has no dominator?");
        exit(1);
      }
      track2track_type key = make_pair( i->first, max_computed_track );
      this->t2t[ key ].target_is_dominated_by_computed = true;
      if (this->verbose)
      {
        LOG_INFO( main_logger, "phase2: target " << scorable_track( i->first ).external_id()
                 << " is dominated by " << scorable_track( max_computed_track ).external_id()
                 << "");
      }
    }
  } // .. for each target track

  // compute dominators for computed tracks
  for ( map< track_handle_type, track_handle_list_type >::const_iterator i = this->c2t.begin();
        i != this->c2t.end();
        ++i )
  {
    track_handle_type max_target_track;
    unsigned max_frames = 0;
    const track_handle_list_type& targets_on_track = i->second;
    if ( ! targets_on_track.empty() )
    {
      for (unsigned j=0; j<targets_on_track.size(); ++j)
      {
        track2track_type key = make_pair( targets_on_track[j], i->first );
        map< track2track_type, track2track_scalars_hadwav >::const_iterator probe = this->t2t.find( key );
        if ( probe == this->t2t.end() )
        {
          LOG_ERROR( main_logger, "Logic error: lost t2c pair?");
          exit(1);
        }
        const track2track_scalars_hadwav& s = probe->second;
        if ( s.computed_frames_on_target == 0 )
        {
          LOG_ERROR( main_logger, "Logic error: t2c contains 0 computed frames on target?");
          exit(1);
        }

        if ( s.computed_frames_on_target > max_frames )
        {
          max_frames = s.computed_frames_on_target;
          max_target_track = targets_on_track[j];
        }
      }
      if ( max_target_track.row == kwiver::track_oracle::INVALID_ROW_HANDLE )
      {
        LOG_ERROR( main_logger, "Logic error: t2c entry has no dominator?");
        exit(1);
      }
      track2track_type key = make_pair( max_target_track, i->first );
      this->t2t[ key ].computed_is_dominated_by_target = true;
      if (this->verbose)
      {
        unsigned id = scorable_track( i->first ).external_id();
        unsigned max_id = scorable_track( max_target_track ).external_id();
        LOG_INFO( main_logger, "phase2: (computed) track " << id << " is dominated by " << max_id << "");
      }
    }
  } // .. for each computed track

  scorable_track_type trk;
  map< ts_type, bool > gt_frame_map, ct_frame_matched_map, ct_frame_unmatched_map, ct_frame_outside_aoi_map;

  track_field< kwiver::track_oracle::dt::utility::state_flags > state_flags;

  for (unsigned i = 0; i < t.size(); ++i )
  {
    const frame_handle_list_type& frames = track_oracle_core::get_frames( t[i] );
    for (unsigned j = 0; j < frames.size(); ++j )
    {
      unsigned match_state = trk[ frames[j] ].frame_has_been_matched();
      if ( ( match_state == IN_AOI_UNMATCHED ) ||
           ( match_state == IN_AOI_MATCHED ))
      {
        gt_frame_map[ trk[frames[j]].timestamp_usecs() ] = true;
      }
    }
  }

  unsigned n_comp_tracks_no_match = 0;
  for (unsigned i = 0; i < c.size(); ++i )
  {
    bool this_track_matched = false;
    const frame_handle_list_type& frames = track_oracle_core::get_frames( c[i] );
    for (unsigned j = 0; j < frames.size(); ++j )
    {
      ts_type this_ts = trk[ frames[j] ].timestamp_usecs();
      switch (trk[ frames[j] ].frame_has_been_matched() )
      {
      case IN_AOI_MATCHED:
        state_flags( frames[j].row ).set_flag( "in-aoi", "true" );
        state_flags( frames[j].row ).set_flag( "matched", "true" );
        ct_frame_matched_map[ this_ts ] = true;
        this_track_matched = true;
        break;

      case IN_AOI_UNMATCHED:
        state_flags( frames[j].row ).set_flag( "in-aoi", "true" );
        state_flags( frames[j].row ).set_flag( "matched", "false" );
        ct_frame_unmatched_map[ this_ts ] = true;
        break;

      case OUTSIDE_AOI:
        state_flags( frames[j].row ).set_flag( "in-aoi", "false" );
        state_flags( frames[j].row ).set_flag( "matched", "n/a" );
        ct_frame_outside_aoi_map[ this_ts ] = true;
        break;

      default:
        throw runtime_error( "Unhandled frame match flag" );
      }
    }
    if ( ! this_track_matched )
    {
      ++n_comp_tracks_no_match;
    }
  }

  int num_gt_frames = gt_frame_map.size();
  int num_comp_frames_with_unique_associations = ct_frame_matched_map.size();
  int num_comp_frames_with_no_associations = ct_frame_unmatched_map.size();
  int num_comp_frames = num_comp_frames_with_unique_associations + num_comp_frames_with_no_associations;

  LOG_INFO( main_logger, "n-gt-detections: " << total_gt_boxes );
  LOG_INFO( main_logger, "n-comp-detections: " << total_computed_boxes );
  LOG_INFO( main_logger, "n-gt-frames:  " << num_gt_frames );
  LOG_INFO( main_logger, "n-comp-frames: " << num_comp_frames );
  LOG_INFO( main_logger, "n-comp-frames-unique-match: " << num_comp_frames_with_unique_associations );
  LOG_INFO( main_logger, "n-comp-frames-no-match: " << num_comp_frames_with_no_associations );
  LOG_INFO( main_logger, "n-comp-frames-outside-aoi: " << ct_frame_outside_aoi_map.size() );

  this->framePD = (num_gt_frames == 0) ? 0.0 : 1.0 * num_comp_frames_with_unique_associations / num_gt_frames;
  this->frameFA = 1.0 * num_comp_frames_with_no_associations;
  this->trackFramePrecision = (num_comp_frames == 0) ? 0.0 : 1.0 * num_comp_frames_with_unique_associations / num_comp_frames;
  this->detectionPD = (total_gt_boxes == 0) ? 0.0 : (1.0 * detected_gt_boxes / total_gt_boxes);
  this->detectionPFalseAlarm = (total_computed_boxes == 0) ? 0.0 : 1.0 * this->detectionFalseAlarms / total_computed_boxes;

}


void
track2track_phase2_hadwav
::debug_dump( ostream& os )
{
  // write out header

  os << "# gt|ct : trk-id trk-len frm-id : n-matches : match-1-trk-id match-1-frm-id [ ; match-2-trk-id match-2-frm-id ... ]\n";

  // first, matches to ground truth
  for ( t2t_it i = this->t2c.begin(); i != this->t2c.end(); ++i )
  {
    debug_dump_entry( os,
                      this->t2t,
                      "gt",
                      i,
                      /* src_is_computed */ false );
    os << "\n";
  }

  // next, all computed tracks
  for ( t2t_it i = this->c2t.begin(); i != this->c2t.end(); ++i )
  {
    debug_dump_entry( os,
                      this->t2t,
                      "ct",
                      i,
                      /* src_is_computed */ true );
    os << "\n";
  } // all computed tracks
}

} // ...kwant
} // ...kwiver


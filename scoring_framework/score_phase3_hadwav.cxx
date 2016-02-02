/*ckwg +5
 * Copyright 2010-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "score_tracks_hadwav.h"

#include <track_oracle/state_flags.h>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::make_pair;
using std::map;
using std::ostringstream;

namespace kwiver {
namespace kwant {

per_track_phase3_hadwav
overall_phase3_hadwav
::compute_per_track( p2it p, const track2track_phase2_hadwav& p2_results, bool seeking_across_truth )
{
  static scorable_track_type local_track_view;

  unsigned dominant_size = 0;
  track_handle_type dominant_index;
  for (unsigned i=0; i<p->second.size(); ++i)
  {
    track_handle_type this_index = p->second[i];
    track2track_type key;
    //
    // TODO: make this semantically consistent
    //
    if ( ! seeking_across_truth)
    {
      key = make_pair( this_index, p->first );
    }
    else
    {
      key = make_pair( p->first, this_index );
    }
    map< track2track_type, track2track_scalars_hadwav >::const_iterator j = p2_results.t2t.find( key );
    if ( j == p2_results.t2t.end() )
    {
      LOG_ERROR( main_logger, "Logic error: phase 3 deduced key of " << key.first.row << ", " << key.second.row << " not in phase 2 results?");
      throw("Whoops");
    }
    unsigned this_size = j->second.computed_frames_on_target;
    if (( i == 0 ) || ( this_size > dominant_size ))
    {
      dominant_size = this_size;
      dominant_index = this_index;
    }
  }

  per_track_phase3_hadwav stats;
  stats.continuity = p->second.size();
  unsigned lifetime = local_track_view( p->first ).frames_in_aoi();
  //  LOG_INFO( main_logger, "dominant size, lifetime: " << dominant_size << "," << lifetime << "");
  stats.purity = (lifetime == 0) ? 0.0 : 1.0*dominant_size / lifetime;
  stats.dominant_track_id = local_track_view( dominant_index ).external_id();
  stats.dominant_track_size = dominant_size;
  stats.dominated_track_lifetime = lifetime;
  // MITRE definition is "over the life of the given {track,target}", implying
  // it's okay to cap this at 1.0
  if (stats.purity > 1.0) stats.purity = 1.0;
  if ( this->verbose )
  {
    LOG_INFO( main_logger, "C/P of ");
    if (seeking_across_truth) LOG_INFO( main_logger, "target "); else LOG_INFO( main_logger, "track ");
    unsigned int domid = local_track_view( dominant_index ).external_id();
    LOG_INFO( main_logger, local_track_view( p->first ).external_id()
             << " (dominated by " << domid << "; size " << dominant_size << "; lifetime " << lifetime << ")"
             << " : cont " << stats.continuity
             << " purity " << stats.purity << "");
  }

  return stats;
}



void
overall_phase3_hadwav
::compute( const track2track_phase2_hadwav& t2t )
{
  // compute MITRE's "track" metrics (i.e. computed tracks)
  map<ts_type, int> numCTOnFrame;
  unsigned purity_counter = 0;
  unsigned continuity_counter = 0;
  for ( p2it iter = t2t.c2t.begin(); iter != t2t.c2t.end(); ++iter )
  {
    per_track_phase3_hadwav stats = this->compute_per_track( iter, t2t, /* looping over truth = */ false  );
    this->mitre_tracks[ iter->first ] = stats;
    if( stats.continuity != 0 )
    {
      this->avg_track_continuity += stats.continuity;
      continuity_counter++;
    }
    if( stats.purity != 0 )
    {
      this->avg_track_purity += stats.purity;
      purity_counter++;
    }
  }
  if ( t2t.n_computed_tracks > 0 )
  {
    if (continuity_counter > 0)
    {
      this->avg_track_continuity /= static_cast<double>( continuity_counter );
    }
    if (purity_counter > 0)
    {
      this->avg_track_purity /= static_cast<double>( purity_counter );
    }
  }
  LOG_INFO( main_logger, "CP (track) avg over " << t2t.n_computed_tracks << "");

  // compute MITRE's "target" metrics (i.e. ground truth )
  map<ts_type, int> numGTOnFrame;
  for ( p2it iter = t2t.t2c.begin(); iter != t2t.t2c.end(); ++iter )
  {
    per_track_phase3_hadwav stats = this->compute_per_track( iter, t2t, /* looping over truth = */ true  );
    this->mitre_targets[ iter->first ] = stats;
    this->avg_target_continuity += stats.continuity;
    this->avg_target_purity += stats.purity;
  }
  if ( t2t.n_true_tracks > 0 )
  {
    this->avg_target_continuity /= (1.0 * t2t.n_true_tracks );
    this->avg_target_purity /= (1.0 * t2t.n_true_tracks );
  }
  LOG_INFO( main_logger, "CP (target) avg over " << t2t.n_computed_tracks << "");

  track_field< dt::utility::state_flags > state_flags;

  // compute overall Pd/FA (or FAR)
  unsigned n_hit_true_tracks = 0;
  for (p2it iter = t2t.t2c.begin(); iter != t2t.t2c.end(); ++iter )
  {
    {
      ostringstream oss;
      oss << iter->second.size();
      state_flags( iter->first.row).set_flag( "n-matched", oss.str() );
    }

    if ( ! iter->second.empty() )
    {
      ++n_hit_true_tracks;
    }

  }
  unsigned n_unassigned_computed_tracks = 0;
  LOG_INFO( main_logger, "t2t.c2t is " << t2t.c2t.size() << "");
  for (p2it iter = t2t.c2t.begin(); iter != t2t.c2t.end(); ++iter )
  {
    {
      ostringstream oss;
      oss << iter->second.size();
      state_flags( iter->first.row).set_flag( "n-matched", oss.str() );
    }
    if ( this->verbose )
    {
      LOG_INFO( main_logger, "FAR: computed " << iter->first.row  << " has " << iter->second.size() << "");
    }
    if ( iter->second.empty() ) ++n_unassigned_computed_tracks;
  }
  LOG_INFO( main_logger, "trackPD: " << n_hit_true_tracks << " / " << t2t.n_true_tracks << "");
  this->trackPd = (t2t.t2c.empty()) ? 0.0 : 1.0 * n_hit_true_tracks / t2t.n_true_tracks;
  LOG_INFO( main_logger, "trackFA: " << n_unassigned_computed_tracks << "");
  this->trackFA = (t2t.c2t.empty()) ? 0.0 : 1.0 * n_unassigned_computed_tracks;
}

const map< track_handle_type, per_track_phase3_hadwav >&
overall_phase3_hadwav
::get_mitre_track_stats() const
{
  return this->mitre_tracks;
}

const map< track_handle_type, per_track_phase3_hadwav >&
overall_phase3_hadwav
::get_mitre_target_stats() const
{
  return this->mitre_targets;
}

} // ...kwant
} // ...kwiver

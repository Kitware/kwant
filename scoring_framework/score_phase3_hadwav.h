/*ckwg +5
 * Copyright 2010-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_SCORE_PHASE3_HADWAV_H
#define INCL_SCORE_PHASE3_HADWAV_H

// In phase 3, the actual metrics are produced.

#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <scoring_framework/score_core.h>
#include <scoring_framework/score_tracks_hadwav_export.h>

namespace kwiver {
namespace kwant {

namespace kwto = ::kwiver::track_oracle;

struct track2track_phase2_hadwav;

struct SCORE_TRACKS_HADWAV_EXPORT per_track_phase3_hadwav
{
public:
  double continuity;
  double purity;
  unsigned dominant_track_id;
  unsigned dominant_track_size;
  unsigned dominated_track_lifetime;
  per_track_phase3_hadwav()
    : continuity(0.0), purity(0.0), dominant_track_id(0), dominant_track_size(0), dominated_track_lifetime(0)
  {}
};

struct SCORE_TRACKS_HADWAV_EXPORT overall_phase3_hadwav
{
private:
  typedef std::map< kwto::track_handle_type, kwto::track_handle_list_type >::const_iterator p2it;

  std::map< kwto::track_handle_type, per_track_phase3_hadwav > mitre_tracks;
  std::map< kwto::track_handle_type, per_track_phase3_hadwav > mitre_targets;

public:
  double avg_track_continuity;
  double avg_track_purity;
  double avg_target_continuity;
  double avg_target_purity;
  double trackPd;
  double trackFA;
  bool verbose;

  overall_phase3_hadwav():
    avg_track_continuity(0.0),
    avg_track_purity(0.0),
    avg_target_continuity(0.0),
    avg_target_purity(0.0),
    trackPd(0.0),
    trackFA(0.0),
    verbose(false)
    {
    }

  per_track_phase3_hadwav compute_per_track( p2it p, const track2track_phase2_hadwav& t2t, bool seeking_across_truth );
  void compute( const track2track_phase2_hadwav& t2t );
  const std::map< kwto::track_handle_type, per_track_phase3_hadwav >& get_mitre_track_stats() const;
  const std::map< kwto::track_handle_type, per_track_phase3_hadwav >& get_mitre_target_stats() const;
};

} // ...kwant
} // ...kwiver

#endif

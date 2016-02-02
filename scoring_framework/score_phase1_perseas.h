/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_SCORE_PERSEAS_PHASE1_H
#define INCL_SCORE_PERSEAS_PHASE1_H

#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <limits>
#include <track_oracle/scoring_framework/score_core.h>
#include <track_oracle/scoring_framework/event_phase1_parameters.h>
#include <track_oracle/scoring_framework/score_phase1.h>
#include <track_oracle/scoring_framework/perseas_phase_1_match_score.h>
#include <track_oracle/scoring_framework/activity_phase1_parameters.h>
#include <ostream>

#include <utilities/timestamp.h>

//
// In phase 1, each ground truth event/activity is compared to each computed event/activity
// independently of all other possible pairings and reduced to a matrix
// of event-to-event score objects.  Ideally, all further scoring can
// be based off these objects (plus the optional phase contexts) without
// needing to ever look again at the events themselves.  If you need to
// look at the events, then whatever it was you were looking for should
// probably go in this structure instead.  We are using the result of
// score phase 1 of tracks (i.e. we do not look directly at tracks)
namespace vidtk
{
//

template<class SRC_MATCHES, class RST_MATCHES, class PARAMS>
struct perseas_phase_1
{
public:
  typedef RST_MATCHES result_match_score_type;
  typedef SRC_MATCHES source_match_score_type;
  // key: (gt_handle, computed_handle)   value: resulting t2t_score
  std::map< track2track_type, RST_MATCHES > r2r;
  std::map< track2track_type, SRC_MATCHES > s2s;
  std::map< unsigned int, track_handle_type > id2s;

  value_keys field_values;

  perseas_phase_1()
  {}
  perseas_phase_1( std::map< track2track_type, SRC_MATCHES > const& in_s,
                   std::map< unsigned int, track_handle_type > const & in_map,
                   value_keys fv )
    : s2s(in_s), id2s(in_map), field_values(fv)
  {}
  perseas_phase_1( PARAMS const& new_params,
                   std::map< track2track_type, SRC_MATCHES > const& in,
                   std::map< unsigned int, track_handle_type > const & in_map,
                   value_keys fv )
    : s2s(in), id2s(in_map), field_values(fv), params(new_params)
  {}

  void compute_all( const track_handle_list_type& gt,
                    const track_handle_list_type& cp );

  void compute_all( const track_handle_list_type& gt,
                    const track_handle_list_type& cp,
                    test_match_abstract<typename RST_MATCHES::match_type> const & tma,
                    get_sub_time_interval_helper const & gstih );

  bool compute_single( track_handle_type gt, track_handle_type cp,
                       test_match_abstract<typename RST_MATCHES::match_type> const & tma,
                       get_sub_time_interval_helper const & gstih );

  PARAMS params;

  void write_matches(std::ostream& str,
                     const track_handle_list_type& gt,
                     const track_handle_list_type& cp) const;
};

typedef perseas_phase_1<track2track_score,event2event_score,event_phase1_parameters> event2event_phase1;
typedef perseas_phase_1<event2event_score,activity2activity_score,activity_phase1_parameters> activity2activity_phase1;

template<class PHASE_1_TYPE >
struct comparer_less_than_handles
{
  bool is_gt_sort;
  const PHASE_1_TYPE & p1;
  track_handle_type at;
  bool operator()(const track_handle_type& l, const track_handle_type& r ) const;
  comparer_less_than_handles(bool isgt, const PHASE_1_TYPE & in )
  : is_gt_sort(isgt), p1(in)
  {}
};

}

#endif

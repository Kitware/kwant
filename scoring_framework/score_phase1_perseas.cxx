/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "score_phase1_perseas.txx"
#include <scoring_framework/event_phase1_parameters.h>
#include <scoring_framework/score_phase1.h>
#include <scoring_framework/perseas_phase_1_match_score.h>

namespace vidtk
{

template<>
void
perseas_phase_1<track2track_score,event2event_score,event_phase1_parameters>
::compute_all( const track_handle_list_type& gt,
               const track_handle_list_type& cp )
{
  this->field_values.id = "event_id";
  this->field_values.type = "event_type";
  this->field_values.supporting_ids = "event_supporting_track_ids";
  this->field_values.begin_time = "event_start_time";
  this->field_values.end_time = "event_end_time";
  this->field_values.probability = "event_probability";
  this->field_values.supporting_begins = "event_supporting_track_start_times";
  this->field_values.supporting_ends = "event_supporting_track_end_times";
  test_track_fragment_score ttfs(this->params);
  get_event_time_interval_helper getih;
  this->compute_all(gt,cp,ttfs,getih);
}


template<>
void
perseas_phase_1<event2event_score, activity2activity_score, activity_phase1_parameters>
::compute_all( const track_handle_list_type& gt,
               const track_handle_list_type& cp )
{
  this->field_values.id = "activity_id";
  this->field_values.type = "activity_type";
  this->field_values.supporting_ids = "activity_supporting_event_ids";
  this->field_values.begin_time = "activity_start_time";
  this->field_values.end_time = "activity_end_time";
  this->field_values.probability = "activity_probability";
  this->field_values.supporting_begins = "activity_start_time";
  this->field_values.supporting_ends = "activity_end_time";
  test_event2event_score te2es(this->params);
  get_activity_time_interval_helper gatih;
  this->compute_all(gt,cp,te2es,gatih);
}

}


template struct vidtk::perseas_phase_1<vidtk::track2track_score,vidtk::event2event_score,vidtk::event_phase1_parameters>;
template struct vidtk::comparer_less_than_handles<vidtk::event2event_phase1>;

template struct vidtk::perseas_phase_1< vidtk::event2event_score, vidtk::activity2activity_score, vidtk::activity_phase1_parameters>;
template struct vidtk::comparer_less_than_handles<vidtk::activity2activity_phase1>;

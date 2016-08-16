/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "score_tracks_aipr.h"


using std::map;
using std::vector;

namespace vidtk
{

  bool ranges_overlap(ts_frame_range r1, ts_frame_range r2)
  {
    return (r1.first  >= r2.first && r1.first  <= r2.second) ||
           (r1.second >= r2.first && r1.second <= r2.second) ||
           (r2.first  >= r1.first && r2.first  <= r1.second) ||
           (r2.second >= r1.first && r2.second <= r1.second);
  }

void
track2track_phase2_aipr
::compute( const track_handle_list_type& t, const track_handle_list_type& c, const track2track_phase1& p1 )
{
  // MITRE's "target" == ground truth
  // MITRE's "track" == computed track


  this->n_true_tracks = t.size();
  this->n_computed_tracks = c.size();

  //Initialize t2c and c2t so each ground truth will have an entry even if
  //there are no associated computed tracks. This is needed for phase 3
  track_handle_list_type::const_iterator truth_iter = t.begin();
  for(; truth_iter != t.end(); truth_iter++)
  {
    this->t2c[(*truth_iter)] = track_handle_list_type();
  }
  track_handle_list_type::const_iterator comp_iter = c.begin();
  for(; comp_iter != c.end(); comp_iter++)
  {
    this->c2t[(*comp_iter)] = track_handle_list_type();
  }

  //Add associations but as they come in check old ones to make sure they are not in
  //conflict. If they are in conflict remove the one with the bigger association value
  for(map<track2track_type, track2track_score>::const_iterator p1_t2t_iter = p1.t2t.begin();
    p1_t2t_iter != p1.t2t.end();
    ++p1_t2t_iter)
  {
    //set the value of the first t2t to be the first ground truth found in p1
    this->t2t[p1_t2t_iter->first] = track2track_scalars_aipr();

    //Give each blank association a value and a range
    double sum_of_L2_norms = 0;
    vector<track2track_frame_overlap_record>::const_iterator frame_overlap_iter;
    for(frame_overlap_iter = p1_t2t_iter->second.frame_overlaps.begin();
        frame_overlap_iter != p1_t2t_iter->second.frame_overlaps.end();
        ++frame_overlap_iter)
    {
      sum_of_L2_norms += frame_overlap_iter->centroid_distance;
    }
    track_handle_type t_id = p1_t2t_iter->first.first;
    track_handle_type c_id = p1_t2t_iter->first.second;

    if(p1_t2t_iter->second.spatial_overlap_total_frames > 0)
    {
      this->t2t[p1_t2t_iter->first].association_value = (sum_of_L2_norms/p1_t2t_iter->second.spatial_overlap_total_frames);
      this->t2t[p1_t2t_iter->first].associtaion_range = p1_t2t_iter->second.overlap_frame_range;
      this->t2t[p1_t2t_iter->first].associated_frame_count = p1_t2t_iter->second.spatial_overlap_total_frames;
      this->t2t[p1_t2t_iter->first].computed_associated_with_target = true;

      //check to see if an assoction with this ground truth always exists
      track_handle_list_type::iterator t2c_iter = this->t2c[t_id].begin();
      vector<track2track_type> t2c_ids_to_remove;
      for(;t2c_iter != this->t2c[t_id].end(); t2c_iter++)
      {
        track_handle_type other_cid = *t2c_iter;
        track2track_type other_key;
        other_key.first = t_id;
        other_key.second = other_cid;

        if(ranges_overlap(this->t2t[other_key].associtaion_range,this->t2t[p1_t2t_iter->first].associtaion_range))
        {
          //these comp tracks both are associated with a single groundtruth
          //the lower association score will be the one chosen to track it

          // if the association values are equal, choose the longer track
          bool associate_with_other_key = false;
          if (this->t2t[other_key].association_value < this->t2t[p1_t2t_iter->first].association_value)
          {
            associate_with_other_key = true;
          }
          else if (this->t2t[other_key].association_value == this->t2t[p1_t2t_iter->first].association_value)
          {
            if (this->t2t[other_key].associated_frame_count > this->t2t[p1_t2t_iter->first].associated_frame_count)
            {
              associate_with_other_key = true;
            }
          }

          if( associate_with_other_key )
          {
            this->t2t[p1_t2t_iter->first].association_value = -1;
            this->t2t[p1_t2t_iter->first].computed_associated_with_target = false;
          }
          else
          {
            this->t2t[other_key].association_value = -1;
            this->t2t[other_key].computed_associated_with_target = false;
            //Don't mess with the vector currently being iterated through
            //Remove these id's after the loop is done
            t2c_ids_to_remove.push_back(other_key);
            this->c2t[other_cid].erase(remove( this->c2t[other_cid].begin(), this->c2t[other_cid].end(), t_id ));
          }
        }
      }
      //remove old values if they exist
      vector<track2track_type>::iterator remove_id_iter;
      for(remove_id_iter = t2c_ids_to_remove.begin(); remove_id_iter != t2c_ids_to_remove.end(); ++remove_id_iter)
      {
        track_handle_type tmp_tid = remove_id_iter->first;
        track_handle_type tmp_cid = remove_id_iter->second;
        this->t2c[tmp_tid].erase(remove( this->t2c[tmp_tid].begin(), this->t2c[tmp_tid].end(), tmp_cid ));
      }
    }
    if(this->t2t[p1_t2t_iter->first].association_value != -1)
    {
      this->c2t[c_id].push_back(t_id);
      this->t2c[t_id].push_back(c_id);
    }
    else
    {
      if ( this->c2t.find( c_id ) == this->c2t.end() )
      {
        this->c2t[ c_id ] = track_handle_list_type();
      }
      if ( this->t2c.find( t_id ) == this->t2c.end() )
      {
        this->t2c[ t_id ] = track_handle_list_type();
      }
    }
  }
}

} //namespace vidtk

/*ckwg +5
 * Copyright 2010-2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "score_tracks_aipr.h"

#include <logger/logger.h>


using std::make_pair;
using std::map;


#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __vidtk_logger_auto_score_phase3_aipr_cxx__
VIDTK_LOGGER("score_phase3_aipr_cxx");


namespace vidtk
{

void
overall_phase3_aipr
::compute( const track2track_phase2_aipr& t2t )
{

  map< track_handle_type, track_handle_list_type >::const_iterator c2t_iter;
  map< track_handle_type, track_handle_list_type >::const_iterator t2c_iter;
  int num_gt_with_associations = 0;
  int range_gt_with_associations = 0;
  int range_all_gt = 0;
  int range_comp_with_associations = 0;
  size_t tcf_sum = 0;

  for(c2t_iter = t2t.c2t.begin(); c2t_iter != t2t.c2t.end(); ++c2t_iter)
  {
    track_handle_type c_id = c2t_iter->first;
    track_handle_list_type t_vector = c2t_iter->second;
    scorable_track_type track;
    int c_range = track_oracle::get_n_frames( c_id );
    if(t_vector.size() != 0)
    {
      range_comp_with_associations += c_range;
    }
    if(t_vector.size() > 1)
    {
      this->num_identity_switch += 1;
    }
  }

  for(t2c_iter = t2t.t2c.begin(); t2c_iter != t2t.t2c.end(); ++t2c_iter)
  {
    track_handle_type t_id = t2c_iter->first;
    track_handle_list_type c_vector = t2c_iter->second;
    scorable_track_type track;
    int t_range = track_oracle::get_n_frames( t_id );
    range_all_gt += t_range;
    if(c_vector.size() >= 1)
    {
      num_gt_with_associations++;
      range_gt_with_associations += t_range;
    }

    // the TCF numerator is the sum of amount of overlapping frames with this
    // ground-truth track.
    for (size_t ci = 0; ci < c_vector.size(); ++ci)
    {
      track2track_type key = make_pair( t_id, c_vector[ci] );
      map< track2track_type, track2track_scalars_aipr>::const_iterator probe =
        t2t.t2t.find( key );
      if ( probe != t2t.t2t.end() )
      {
        //        LOG_INFO( "TCF: ci " << ci << " adding " << probe->second.associated_frame_count << "");
        tcf_sum += probe->second.associated_frame_count;
      }
      else
      {
        LOG_ERROR( "AIPR TCF error: truth track missing t2t entry for claimed overlapping computed track");
      }
    }

    //numerator of NTF is the summation of comp associated with a ground truth * the length of the ground truth
    this->normalized_track_fragmentation += c_vector.size() * t_range;
    //add the number unique computed id's
    this->track_fragmentation += c_vector.size();
  }
  double num_computed_tracks = t2t.c2t.size();
  //Don't divide by 0
  if (num_computed_tracks != 0)
  {
    this->identity_switch = static_cast<double> (this->num_identity_switch) / num_computed_tracks;
  }
  else
  {
    this->identity_switch = -1;
  }
  if (num_gt_with_associations != 0)
  {
    //the denominator of track_fragmentation is the number of ground truths with any associations
    this->track_fragmentation /= static_cast<double> (num_gt_with_associations);
  }
  else
  {
    this->track_fragmentation = -1;
  }
  if ( range_gt_with_associations != 0 )
  {
    //the denominator of normalized_track_fragmentation is the frame range of ground truths with any associations
    this->normalized_track_fragmentation /= static_cast<double> (range_gt_with_associations);
  }
  else
  {
    this->normalized_track_fragmentation = -1;
  }
  if ( range_all_gt != 0 )
  {
    //sum of all frame ranges of associated computed tracks over the sum of all frame ranges of ground truth tracks
    // from 2006 paper: tcf =
    //  sum over all ground truth G of:
    //      sum over all computed tracks T overlapping G of:
    //          number of frames in overlap of (T,G)
    //
    //  divded by
    //
    //  sum over all ground truth G of:
    //      length of G
    //
    this->track_completeness_factor = 1.0 * tcf_sum / range_all_gt;
  }
  else
  {
    this->track_completeness_factor = -1;
  }


}

} // namespace vidtk

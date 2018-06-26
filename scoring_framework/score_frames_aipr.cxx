/*ckwg +5
 * Copyright 2010 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "score_frames_aipr.h"

#include <string>
#include <iostream>
#include <fstream>

#include <vil/vil_math.h>
#include <vgl/vgl_area.h>
#include <vgl/vgl_point_3d.h>
#include <vgl/vgl_distance.h>
#include <vgl/vgl_intersection.h>
#include <vgl/vgl_intersection.txx>


using std::map;
using std::vector;

using namespace vidtk;

void get_frame_indexed_map(vector<vidtk::track_sptr>& trks,
                           map< int, vector<vidtk::track_sptr> >& o_trks,
                           int* num_frames,
                           double /*min_overlap_ratio*/ = 0)
{
  vector<vidtk::track_sptr>::iterator iter;
  for(iter = trks.begin(); iter != trks.end(); ++iter)
  {
    vector<track_state_sptr> hist = (*iter)->history();
    vector<track_state_sptr>::iterator iter2;
    for(iter2 = hist.begin(); iter2 != hist.end(); ++iter2)
    {
      track_state_sptr ts = (*iter2);
      int frame_num = ts->time_.frame_number();
      (*num_frames) = ( *num_frames) > frame_num ? (*num_frames) : frame_num+1;
      vidtk::track_sptr to_push = new vidtk::track;
      to_push->set_id((*iter)->id());
      to_push->add_state(ts);
      o_trks[frame_num].push_back(to_push);
    }
  }
}

void compute_frame_based_metrics(vector<frame_based_metrics>& fbm,
                                 vector<vidtk::track_sptr>& /*gt_tracks*/,
                                 vector<vidtk::track_sptr>& /*comp_tracks*/,
                                 map< int, vector<vidtk::track_sptr> > gt_fi_map,
                                 map< int, vector<vidtk::track_sptr> > comp_fi_map,
                                 int num_frames,
                                 double min_overlap_ratio = 0)
{
  vector< map<int, vector<int> > > gt_frame_level_associations;
  vector< map< int, vector<int> > > comp_frame_level_associations;

  //Create frame level associations
  for(int i = 0; i < num_frames; i++)
  {
    vector<vidtk::track_sptr>::iterator iter;
    //Add an assocation map for every frame
    map< int, vector<int> > tmp1;
    gt_frame_level_associations.push_back(tmp1);
    map< int, vector<int> > tmp2;
    comp_frame_level_associations.push_back(tmp2);

    //initialize vectors to 0 in association maps
    for(iter = comp_fi_map[i].begin(); iter != comp_fi_map[i].end(); ++iter)
    {
      vidtk::track_sptr comp = (*iter);
      if(comp_frame_level_associations[i].find(comp->id()) == comp_frame_level_associations[i].end())
      {
        comp_frame_level_associations[i][comp->id()].clear();
      }
    }

    for(iter = gt_fi_map[i].begin(); iter != gt_fi_map[i].end(); ++iter)
    {
      vector<vidtk::track_sptr>::iterator iter2;
      vidtk::track_sptr gt = (*iter);
      //initialize vectors to 0 in association maps
      if(gt_frame_level_associations[i].find(gt->id()) == gt_frame_level_associations[i].end())
      {
        gt_frame_level_associations[i][gt->id()].clear();
      }
      for(iter2 = comp_fi_map[i].begin(); iter2 != comp_fi_map[i].end(); ++iter2)
      {
        vidtk::track_sptr comp = (*iter2);
        //initialize vectors to 0 in association maps
        if(comp_frame_level_associations[i].find(comp->id()) == comp_frame_level_associations[i].end())
        {
          comp_frame_level_associations[i][comp->id()].clear();
        }
        vgl_box_2d<unsigned> intersection = vgl_intersection(gt->history()[0]->amhi_bbox_,comp->history()[0]->amhi_bbox_);

        double overlap_score = 0;
        if( vgl_area( intersection ) != 0)
        {
          overlap_score = ( vgl_area( gt->history()[0]->amhi_bbox_) +
                            vgl_area( comp->history()[0]->amhi_bbox_) -
                            vgl_area( intersection ))
            / vgl_area( intersection );
        }

        if(!intersection.is_empty() && overlap_score >= min_overlap_ratio)
        {
          gt_frame_level_associations[i][gt->id()].push_back(comp->id());
          comp_frame_level_associations[i][comp->id()].push_back(gt->id());
        }
      }
    }
  }

  //compute metrics
  for(int i = 0; i < num_frames; i++)
  {
    map< int,vector<int> >::iterator iter;
    frame_based_metrics to_push;
    to_push.detection_probability_count = 0;
    to_push.detection_probability_presence = 0;
    to_push.false_negatives_count = 0;
    to_push.false_positives_count = 0;
    to_push.merge_fraction = 0;
    to_push.num_merge = 0;
    to_push.num_merge_mult = 0;
    to_push.num_split = 0;
    to_push.num_split_mult = 0;
    to_push.precision_count = 0;
    to_push.precision_presence = 0;
    to_push.split_fraction = 0;
    to_push.true_positives_count = 0;
    to_push.true_positives_presence = 0;
    to_push.gt_with_associations = 0;
    to_push.gt_without_associations = 0;

    fbm.push_back(to_push);
    //Compute false_negatives, true_positives, and splits

    for( iter = gt_frame_level_associations[i].begin(); iter != gt_frame_level_associations[i].end(); ++iter)
    {
      //The number of computed tracks associated with this ground truth
      int num_comp_assoc = (*iter).second.size();

      if(num_comp_assoc == 0)
      {
        fbm[i].false_negatives_count++;
        continue;
      }
      fbm[i].gt_with_associations++;
      //The number of ground truth tracks this ground truth tracks associations are associated with
      int num_gt_assoc = comp_frame_level_associations[i][(*iter).second[0]].size();

      if( num_gt_assoc == 1 && num_comp_assoc == 1)
      {
        fbm[i].true_positives_count++;
        fbm[i].true_positives_presence++;
      }

      if ( num_comp_assoc > 1 )
      {
        fbm[i].true_positives_count++;
        fbm[i].false_positives_count+=num_comp_assoc-1;
        fbm[i].true_positives_presence++;
        fbm[i].num_split++;
        fbm[i].num_split_mult += (num_comp_assoc - 1);
      }
    }
    //compute false positives, false negatives and merges
    for( iter = comp_frame_level_associations[i].begin(); iter != comp_frame_level_associations[i].end(); ++iter)
    {
      //The number of ground truth tracks associated with this ground truth
      int num_gt_assoc = (*iter).second.size();
      if(num_gt_assoc == 0)
      {
        fbm[i].false_positives_count++;
        continue;
      }

      if ( num_gt_assoc > 1 )
      {
        fbm[i].true_positives_count++;
        fbm[i].false_negatives_count+=num_gt_assoc-1;
        fbm[i].true_positives_presence+=num_gt_assoc;
        fbm[i].num_merge++;
        fbm[i].num_merge_mult += (num_gt_assoc - 1);
      }
    }

    //Compute ratios

    fbm[i].gt_without_associations = gt_frame_level_associations[i].size() - fbm[i].gt_with_associations;
    if(fbm[i].gt_with_associations == 0 ||
      gt_frame_level_associations[i].size() == 0 ||
      comp_frame_level_associations[i].size() + fbm[i].num_merge - fbm[i].num_split == 0)
    {
      continue;
    }
    fbm[i].detection_probability_presence = static_cast<double>(fbm[i].true_positives_presence)/gt_frame_level_associations[i].size();
    fbm[i].detection_probability_count= static_cast<double>(fbm[i].true_positives_count)/gt_frame_level_associations[i].size();

    fbm[i].precision_presence = fbm[i].true_positives_presence/static_cast<double>(comp_frame_level_associations[i].size() + fbm[i].num_merge - fbm[i].num_split);
    fbm[i].precision_count = fbm[i].true_positives_presence/static_cast<double>(comp_frame_level_associations[i].size() + fbm[i].num_merge - fbm[i].num_split);

    fbm[i].split_fraction = fbm[i].num_split/static_cast<double>(fbm[i].gt_with_associations);
    fbm[i].merge_fraction = fbm[i].num_merge/static_cast<double>(fbm[i].gt_with_associations);
  }
}

void compute_metrics(vector<frame_based_metrics>& fbm,
                     vector<track_sptr>& gt_tracks,
                     vector<track_sptr>& comp_tracks,
                     double min_overlap_ratio)
{
  //Frame Indexed Maps
  map< int, vector<track_sptr> > gt_fi_map;
  map< int, vector<track_sptr> > comp_fi_map;

  int num_frames = 0;

  get_frame_indexed_map(gt_tracks,gt_fi_map,&num_frames,min_overlap_ratio);
  get_frame_indexed_map(comp_tracks,comp_fi_map,&num_frames,min_overlap_ratio);

  compute_frame_based_metrics(fbm, gt_tracks, comp_tracks, gt_fi_map, comp_fi_map, num_frames,min_overlap_ratio);
}

/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "score_phase1_perseas.h"

#include <track_oracle/event/perseas_event.h>
#include <track_oracle/track_kw18/track_kw18.h>

#include <algorithm>
#include <set>



#include <logger/logger.h>


using std::endl;
using std::make_pair;
using std::map;
using std::ostream;
using std::sort;


#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __vidtk_logger_auto_score_phase1_perseas_txx__
VIDTK_LOGGER("score_phase1_perseas_txx");


#undef DEBUG_OUTPUT_EVENT
//#define DEBUG_OUTPUT_EVENT


namespace vidtk
{

template<class SRC_MATCHES, class RST_MATCHES, class PARAMS>
void
perseas_phase_1<SRC_MATCHES,RST_MATCHES,PARAMS>
::compute_all( const track_handle_list_type& t,
               const track_handle_list_type& c,
               test_match_abstract<typename RST_MATCHES::match_type> const & tma,
               get_sub_time_interval_helper const & gstih )
{
  for ( unsigned i=0; i<t.size(); ++i )
  {
    if ((i % 10 == 0) || (i == t.size()-1)) {
      LOG_INFO( "phase 1: " << i << " of " << t.size() << "..." );
    }
    for (unsigned j=0; j<c.size(); ++j)
    {
      this->compute_single( t[i], c[j], tma, gstih );
    }
  }
}

template<class SRC_MATCHES, class RST_MATCHES, class PARAMS>
bool
perseas_phase_1<SRC_MATCHES,RST_MATCHES,PARAMS>
::compute_single( track_handle_type gt, track_handle_type cp,
                  test_match_abstract<typename RST_MATCHES::match_type> const & tma,
                  get_sub_time_interval_helper const & gstih )
{
  track2track_type key = make_pair( gt, cp );

  // quick exit if we've already computed the score for this pair
  if ( this->r2r.find( key ) != this->r2r.end() )
  {
    return true;
  }

  RST_MATCHES r2r_score;
  bool b = r2r_score.compute( gt, cp, field_values, tma, gstih, s2s, id2s );
  if ( b && test_score(r2r_score, params))
  {
    this->r2r[ key ] = r2r_score;
  }
  return b;
}

template<class SRC_MATCHES, class RST_MATCHES, class PARAMS>
void
perseas_phase_1<SRC_MATCHES,RST_MATCHES,PARAMS>
::write_matches(ostream& str,
                const track_handle_list_type& gt,
                const track_handle_list_type& cp) const
{
  map< track_handle_type, track_handle_list_type > c2t;  // key = computed ID; val = list of associated truth tracks sorted by best score
  map< track_handle_type, track_handle_list_type > t2c;  // key = truth track ID; val = list of associated computed tracks by best score
  for(unsigned int i = 0; i < gt.size(); ++i)
  {
    t2c[gt[i]].clear();
  }
  for(unsigned int i = 0; i < cp.size(); ++i)
  {
    c2t[cp[i]].clear();
  }
  typedef typename map< track2track_type, RST_MATCHES >::const_iterator r2r_iter;
  typedef map< track_handle_type, track_handle_list_type >::iterator t2v_iter;
  for(r2r_iter iter = r2r.begin(); iter != r2r.end(); ++iter)
  {
    t2c[iter->first.first].push_back(iter->first.second);
    c2t[iter->first.second].push_back(iter->first.first);
  }
  comparer_less_than_handles< perseas_phase_1<SRC_MATCHES,RST_MATCHES,PARAMS> > comp(/*is_gt=*/false,*this);
  track_field< unsigned > id_field(field_values.id);
  for(t2v_iter iter = c2t.begin(); iter!= c2t.end(); ++iter)
  {
    comp.at = iter->first;
    sort(iter->second.rbegin(), iter->second.rend(), comp);
    str << "CP: "<< id_field(iter->first.row) << " :";
    for(track_handle_list_type::const_iterator i2 =iter->second.begin(); i2 != iter->second.end(); ++i2 )
    {
      str << " " << id_field(i2->row);
    }
    str << endl;
  }
  comp.is_gt_sort = true;
  for(t2v_iter iter = t2c.begin(); iter!= t2c.end(); ++iter)
  {
    comp.at = iter->first;
    sort(iter->second.rbegin(), iter->second.rend(), comp);
    str << "GT: "<< id_field(iter->first.row) << " :";
    for(track_handle_list_type::const_iterator i2 =iter->second.begin(); i2 != iter->second.end(); ++i2 )
    {
      str << " " << id_field(i2->row);
    }
    str << endl;
  }
}

template<class PHASE_1_TYPE>
bool
comparer_less_than_handles<PHASE_1_TYPE>
::operator()(const track_handle_type& l, const track_handle_type& r ) const
{
  track2track_type key_l = (is_gt_sort)?track2track_type( at, l):track2track_type( l, at);
  typedef typename map< track2track_type, typename PHASE_1_TYPE::result_match_score_type >::const_iterator cst_iter;
  cst_iter l_score = p1.r2r.find(key_l);
  if(l_score == p1.r2r.end())
  {
    LOG_ERROR("There should be an lscore" );
    return false;
  }
  track2track_type key_r = (is_gt_sort)?track2track_type( at, r ):track2track_type( r, at );
  cst_iter r_score = p1.r2r.find(key_r);
  if(r_score == p1.r2r.end())
  {
    LOG_ERROR("There should be a r score" );
    return false;
  }
  return compare_less_than< typename PHASE_1_TYPE::result_match_score_type>::less_than( l_score->second,
                                                                                        r_score->second,
                                                                                        is_gt_sort);
}

}//namepase vidtk

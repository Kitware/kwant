/*ckwg +5
 * Copyright 2011-2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "score_tracks_loader.h"

#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <limits>
#include <stdexcept>

#include <vul/vul_file.h>
#include <vul/vul_reg_exp.h>

#include <vgl/vgl_area.h>

#include <scoring_framework/score_core.h>
#include <track_oracle/aries_interface/aries_interface.h>
#include <track_oracle/file_formats/track_kwxml/file_format_kwxml.h>
#include <track_oracle/file_formats/track_kw18/file_format_kw18.h>
#ifdef SHAPELIB_ENABLED
#include <track_oracle/file_formats/track_apix/file_format_apix.h>
#endif
#include <track_oracle/file_formats/track_xgtf/file_format_xgtf.h>
#include <track_oracle/file_formats/track_vpd/track_vpd_event.h>
#include <track_oracle/file_formats/track_vpd/file_format_vpd_track.h>
#include <track_oracle/file_formats/track_comms_xml/file_format_comms_xml.h>
#include <track_oracle/file_formats/file_format_manager.h>
#ifdef KWANT_ENABLE_MGRS
#include <track_oracle/file_formats/track_scorable_mgrs/track_scorable_mgrs.h>
#endif
#include <track_oracle/file_formats/file_format_schema.h>

#include <scoring_framework/time_window_filter.h>
#include <scoring_framework/timestamp_utilities.h>
#include <scoring_framework/virat_scenario_utilities.h>
#include <track_oracle/core/state_flags.h>

#include <tinyxml.h>

#include <boost/lexical_cast.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <utilities/shell_comments_filter.h>
#include <utilities/blank_line_filter.h>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::endl;
using std::exit;
using std::getline;
using std::ifstream;
using std::istringstream;
using std::make_pair;
using std::map;
using std::ofstream;
using std::ostringstream;
using std::pair;
using std::runtime_error;
using std::setprecision;
using std::string;
using std::vector;

using kwiver::track_oracle::frame_handle_list_type;
using kwiver::track_oracle::frame_handle_type;
using kwiver::track_oracle::track_handle_list_type;
using kwiver::track_oracle::track_handle_type;
using kwiver::track_oracle::track_oracle_core;
using kwiver::track_oracle::file_format_schema_type;
using kwiver::track_oracle::file_format_manager;
using kwiver::track_oracle::track_vpd_track_type;
using kwiver::track_oracle::track_vpd_event_type;
using kwiver::track_oracle::track_comms_xml_type;
using kwiver::track_oracle::comms_xml_reader_opts;
using kwiver::track_oracle::kwxml_reader_opts;
using kwiver::track_oracle::xgtf_reader_opts;
using kwiver::track_oracle::track_field;
using kwiver::track_oracle::oracle_entry_handle_type;
using kwiver::track_oracle::aries_interface;
using kwiver::track_oracle::track_base_impl;

using namespace kwiver::kwant;
using namespace kwiver::kwant::timestamp_utilities;

class track_record_type
{
public:
  track_record_type() {}

  const string& src_fn() const { return this_src_fn; }
  void set_src_fn( const string& s ) { this_src_fn = s; }

  const track_handle_list_type& tracks() const { return this_tracks; }
  void set_tracks( const track_handle_list_type& t )
  {
    this_tracks = t;
    recompute_stats();
  }

  void recompute_stats()
  {
    this_stats.reset();
    this_stats.set_from_tracks( this_tracks );
  }

  const track_timestamp_stats_type& stats() const { return this_stats; }
  bool timestamp_overlaps( const track_record_type& other ) const;

private:
  string this_src_fn;
  track_handle_list_type this_tracks;
  track_timestamp_stats_type this_stats;
};

//
// This object is the expansion of the track source argument, for both
// ground-truth and computed tracks.  If timestamp_generator_map is
// empty, then the source time already had timestamps (e.g. kw18).
//
// When the input file is a VIRAT scenario, also return a map of
// query_id -> activity_list.  When the other input is a comms_xml
// from the test harness, which only has query_ids, we can use this
// map to convert to activities.
//

struct input_source_type
{
  input_source_type( const string& arg, vul_arg< string >& path_override, double fps, size_t min_track_length_param );
  vector< string > fn_list;
  timestamp_generator_map_type timestamp_generator_map;
  map< string, vector< size_t > > qid2activity_map;
  size_t min_track_length;
};

///
/// Until the alignment relationship branch lands, the quick way
/// to support scoring file formats which don't contain timestamps
/// is to use the --fn2ts flag to convert frame numbers to timestamps.
/// Converts frame numbers straight into seconds.  (If the format
/// doesn't support frame numbers, log an error but do nothing else;
/// the caller will decide if it's worth it to carry on.)
///
/// There are a couple of ways to do this.  One would be to look up
/// the source format of the track, obtain an instance its schema,
/// and see if it contains the timestamp and frame number fields.
///
/// The other way is to directly query the tracks to see if they contain
/// the timestamp / frame number fields.  (Rather, query the first track
/// in the record and assume the answer holds for the other tracks.)
///
/// I went with the second way because it works better with the idea of
/// starting with a source format and "decorating" it with the fields as
/// we move down the processing chain.  This way, for example, you can
/// call this routine twice in a row on the same data and it does what you'd
/// expect (add timestamps the first time, do nothing the second time.)
/// The first way, it would add the timestamps redundantly the second time.
///


void
promote_frame_number_to_timestamp( vector< track_record_type >& tr )
{
  scorable_track_type stt;

  for (size_t i=0; i<tr.size(); ++i)
  {
    // need to find a frame to examine; assume that what holds for
    // one frame will hold for all
    track_record_type& r = tr[i];
    const track_handle_list_type& r_tracks = r.tracks();
    if (r_tracks.empty()) continue;

    frame_handle_list_type frames;
    for (size_t j=0; j<r_tracks.size() && frames.empty(); ++j)
    {
      frames = track_oracle_core::get_frames( r_tracks[j] );
    }
    // couldn't find any frames... nothing to promote!
    if (frames.empty()) continue;

    // if the frame does NOT define frame numbers, log an error and bail
    if ( ! stt.timestamp_frame.exists( frames[0].row ))
    {
      LOG_ERROR( main_logger, "frame-number-to-timestamp promotion requested for " << r.src_fn()
                 << ", but format does not does not contain frame numbers; skipping" );
      continue;
    }

    // if we get this far, we (a) don't have a timestamp, but (b) have frame numbers.
    // Loop over all tracks / frames and plug in timestamps
    size_t c=0, total_f=0;
    for (size_t j=0; j<r_tracks.size(); ++j)
    {
      frames = track_oracle_core::get_frames( r_tracks[j] );
      for (size_t f=0; f<frames.size(); ++f)
      {
        ++total_f;
        if (stt.timestamp_frame.exists( frames[f].row ))
        {
          ts_type new_timestamp = stt[ frames[f] ].timestamp_frame() * 1000 * 1000;
          stt[ frames[f] ].timestamp_usecs() = new_timestamp;
          ++c;
        }
      }
    }
    LOG_INFO( main_logger, "frame-number-to-timestamp for " << r.src_fn() << ": promoted "
              << c << " of " << total_f << " frames" );
		// reset the all-have-timestamps flag
		r.recompute_stats();
  } // ... for each element in the track record
}

///
/// The VIRAT Public Data "object tracks" don't have an external ID, but
/// they do have an object ID.  One might argue that this should've been
/// directly coded as the external ID in the first place, but on the assumption
/// that the schemas should hew as closely as possible to the external
/// definition of the format, and clients (such as us) are responsible for
/// any conversions required for our application, this method goes down the
/// track records and copies any object IDs over to external IDs.
///
/// It is tempting to generalize both this and the fn2ts routine to a more
/// general transformation paradigm, but we'll defer that for now.
///


void
promote_object_id_to_external_id( const vector< track_record_type >& tr )
{
  file_format_schema_type ffs;
  track_vpd_track_type vpd;
  scorable_track_type stt;

  for (size_t i=0; i<tr.size(); ++i)
  {
    const track_record_type& r = tr[i];
    const track_handle_list_type& r_tracks = r.tracks();
    if (r_tracks.empty()) continue;

    // only process TF_VPD_TRACK types
    if ( ffs( r_tracks[0] ).format() != kwiver::track_oracle::TF_VPD_TRACK ) continue;

    for (size_t j=0; j<r_tracks.size(); ++j)
    {
      stt( r_tracks[j] ).external_id() = vpd( r_tracks[j] ).object_id();
    }
  }
}

void
promote_vpd_event_to_virat_event( const vector< track_record_type >& tr )
{
  file_format_schema_type ffs;
  track_vpd_event_type vpd;

  track_field<int> virat_activity_index( "activity" );
  track_field<double> virat_activity_probability( "activity_probability" );

  size_t prob_to_one_count = 0;
  size_t activity_promotion_count = 0;

  for (size_t i=0; i<tr.size(); ++i)
  {
    const track_record_type& r = tr[i];
    const track_handle_list_type& r_tracks = r.tracks();
    if (r_tracks.empty()) continue;

    // only process TF_VPD_TRACK types
    if ( ffs( r_tracks[0] ).format() != kwiver::track_oracle::TF_VPD_TRACK ) continue;

    for (size_t j=0; j<r_tracks.size(); ++j)
    {
      oracle_entry_handle_type row = r_tracks[j].row;
      pair< bool, unsigned > vpd_event_probe = vpd.event_type.get( row );
      if ( ! vpd_event_probe.first) continue;

      string n = aries_interface::vpd_index_to_activity( vpd_event_probe.second );
      if ( n == "" )
      {
        LOG_WARN( main_logger, "VPD index " << vpd_event_probe.second  << " has no VIRAT equivalent" );
        continue;
      }
      virat_activity_index( row ) = aries_interface::activity_to_index( n );
      ++activity_promotion_count;

      // If we don't already have a probability, assume this is loaded as ground truth
      // and set the probability to one

      if ( ! virat_activity_probability.exists( row ))
      {
        virat_activity_probability( row ) = 1.0;
        ++prob_to_one_count;
      }
    }
  }
  if (activity_promotion_count > 0)
  {
    LOG_INFO( main_logger, "VPD->VIRAT event promotion: assumed probability == 1 for " << prob_to_one_count
              << " out of " << activity_promotion_count << " tracks" );
  }
}



///
/// We now support THREE ways of obtaining a set of track files on the command
/// line:
///
/// 1) a single filename: "-gt filename.kw18"
///
/// 2) a file which in turn contains other filenames: "-gt @groundtruth-list.txt"
///
/// 3) a VIRAT scenario file (which also contains timestamps) "-gt scenario.xml"
///
///
/// ...and with the truth_path / computed_path options as well.
///

string
resolve_filename( const string& given_path,
                  vul_arg< string >& path_override )
{
  if ( path_override() == "" ) return given_path;

  string base_path = vul_file::basename( given_path );
  string final_path = path_override()+"/"+base_path;
  if ( base_path != given_path )
  {
    LOG_INFO( main_logger, "Overriding " << given_path << " to " << final_path );
  }
  return final_path;
}


input_source_type
::input_source_type( const string& arg,
                     vul_arg< string >& path_override,
                     double fps,
                     size_t min_track_length_param )
  : min_track_length( min_track_length_param )
{
  // Either arg starts with "@", in which case it's a list,
  // or it can be loaded as a scenario file,
  // else it will be interpreted downstream as a filename.
  // Only load the timestamps if it's a scenario file.
  // empty vector of filenames signals error.

  if (arg.empty()) return;

  if (arg[0] == '@')
  {
    //
    // Case 1: argument is a pointer to a list of filenames,
    // which are read without further interpretation.
    //
    string fn = arg.substr(1, arg.size() - 1);
    ifstream is( fn.c_str() );
    if ( ! is )
    {
      LOG_ERROR( main_logger, "Couldn't open filename list '" << fn << "'");
      this->fn_list.clear();
      return;
    }

    // build boost filter to remove comments and blank lines
    boost::iostreams::filtering_istream in_stream;
    in_stream.push (vidtk::blank_line_filter());
    in_stream.push (vidtk::shell_comments_filter());
    in_stream.push (is);

    string tmp;
    while (getline(in_stream, tmp ))
    {
      this->fn_list.push_back( resolve_filename( tmp, path_override ));
    }
  }
  else
  {
    // It's a single string; does that string name a VIRAT scenario?

    if ( ! virat_scenario_utilities::fn_is_virat_scenario( arg ))
    {
      //
      // Case 2: Doesn't start with '@', can't load as a scenario;
      // treat as a single filename.
      //
      this->fn_list.push_back( resolve_filename( arg, path_override ));
    }
    else
    {
      //
      // Case 3: it seems to be a VIRAT scenario file.
      //

      timestamp_generator_map_type full_pathname_map;
      if ( ! virat_scenario_utilities::process( arg, this->qid2activity_map, full_pathname_map, fps ) )
      {
        return;
      }
      // populate fn_list with the keys of the timestamp_generator_map
      for (timestamp_generator_cit i = full_pathname_map.begin();
           i != full_pathname_map.end();
           ++i)
      {
        this->fn_list.push_back( resolve_filename( i->first, path_override ));
        this->timestamp_generator_map[ vul_file::basename( i->first ) ] = i->second;
      }

    } // ... conclude case 2 vs 3
  } // ... conclude case 1 vs 2/3
  // all done!
}

#ifdef KWANT_ENABLE_MGRS
//
// MGRS data will be appended if:
// (a) a track is APIX, regardless of whether or not the user asked for it,
// (b) the track is any other format, if the user asked for it.
//
// Note that if the user asked for it, but set_from_tracklist fails, we throw
// an exception.
//
// Otherwise, this routine is a no-op.
//

void
set_mgrs_data( const vector< track_record_type >& tr_list,
               bool user_requested_mgrs_conversion,
               const string& lon_field,
               const string& lat_field )
{
  file_format_schema_type tfs;

  for (size_t i=0; i<tr_list.size(); ++i)
  {
    const track_handle_list_type& r_tracks = tr_list[i].tracks();
    if (r_tracks.empty()) continue;
    const track_handle_type& r0 = r_tracks[0];
    string fn = file_format_schema_type::source_id_to_filename( tfs( r0 ).source_file_id() );

    bool add_to_this_record = false;
    if (tfs( r0 ).format() == kwiver::track_oracle::TF_APIX)
    {
      if ( ! user_requested_mgrs_conversion )
      {
        LOG_INFO( main_logger, "No user-requested MGRS conversion for APIX file '" << fn << "'; adding MGRS anyway" );
      }
      add_to_this_record = true;
    }
    else
    {
      add_to_this_record = user_requested_mgrs_conversion;
    }
    if ( ! add_to_this_record ) continue;

    if ( ! track_scorable_mgrs_type::set_from_tracklist( r_tracks, lon_field, lat_field ))
    {
      ostringstream oss;
      oss << "Could not add MGRS data to tracks from '" << fn << "'";
      LOG_ERROR( main_logger, oss.str() );
      throw runtime_error( oss.str().c_str() );
    }
    LOG_INFO( main_logger, "Added MGRS data to " << r_tracks.size() << " tracks from " << fn );
  }
}
#endif

#ifdef SHAPELIB_ENABLED
//
// Two things need to be done to APIX tracks:
// 1) their timestamps need to be converted from vidtk timestamps to timestamp_usecs
// 2) Per Juda's request, optionally log how we read them
//

void
convert_apix_timestamps( vector< track_record_type >& tr_list,
                         const string& dbg_fn )
{
  file_format_schema_type tfs;
  track_apix_type apix_schema;
  track_field< ts_type > timestamp_usecs( "timestamp_usecs" );

  ofstream* os = 0;
  for (size_t i=0; i<tr_list.size(); ++i)
  {
    const track_handle_list_type& r_tracks = tr_list[i].tracks();
    if (r_tracks.empty()) continue;
    const track_handle_type& r0 = r_tracks[0];

    if ( tfs( r0 ).format() != kwiver::track_oracle::TF_APIX ) continue;

    string fn = file_format_schema_type::source_id_to_filename( tfs( r0 ).source_file_id() );

    if ((dbg_fn != "") && ( ! os ))
    {
      os = new ofstream();
      os->open( dbg_fn.c_str(), ofstream::out | ofstream::app );
      if ( ! (*os) )
      {
        ostringstream oss;
        oss << "Couldn't append to APIX debug log file '" << dbg_fn << "'";
        throw runtime_error( oss.str() );
      }
      (*os) << setprecision(9);
    }
    for (size_t j = 0; j<r_tracks.size(); ++j)
    {
      frame_handle_list_type frames = track_oracle_core::get_frames( r_tracks[j] );

      for (size_t f = 0; f<frames.size(); ++f )
      {
        frame_handle_type frame = frames[f];

        // convert timestamp
        const timestamp& ts = apix_schema[ frame ].utc_timestamp();
        if ( ! ts.has_time() )
        {
          ostringstream oss;
          oss << "APIX track in " << fn << " id " << apix_schema[frame].external_id()
              << " @ time " << ts << " has no time?";
          throw runtime_error( oss.str() );
        }
        timestamp_usecs( frame.row ) = static_cast< ts_type >( ts.time() );

        // optionally log info
        if (os)
        {
          (*os) << fn << " track " << apix_schema[ frame ].external_id()
                << " frame " << f << " @ " << ts << " : lon " << apix_schema[ frame ].lon()
                << " ; lat " << apix_schema[ frame ].lat()
                << endl;
        }
      } // ...each frame
    } // .. each track
  } // ...each track record

  delete os;

}
#endif

void
set_virat_scenario_activities( const input_source_type& truth_src,
                               vector< track_record_type >& computed_track_records )
{
  // quick exit if truth_src has no qid2activity records and computed_track_records are not comm files.
  if (computed_track_records.empty()) return;
  if (computed_track_records[0].tracks().empty()) return;
  track_handle_type track0 = computed_track_records[0].tracks()[0];
  file_format_schema_type ffs;
  if ( ffs( track0 ).format() != kwiver::track_oracle::TF_COMMS_XML ) return;
  if ( truth_src.qid2activity_map.empty() )
  {
    LOG_WARN( main_logger, string("\n")+
              "Computed tracks are from a comms file but no query IDs were found in\n"+
              "the truth tracks. The computed tracks will contain no activity labels\n"+
              "and can't be used for event scoring." );
    return;
  }

  LOG_INFO( main_logger, "VIRAT scenario / comms: populating activity information from scenario queries" );

  //
  // This is a little ugly.  The standard paradigm inherited from XGTF
  // is: one-to-one mapping between an activity and a track.  If a
  // single comms query result, from a single query ID, has multiple
  // activities in its corresponding entry in the scenario file, we
  // copy the comms track here and rely on score_events upstream
  // filtering to make the numbers all work out.

  track_comms_xml_type comms, comms_copy;
  track_field< int > activity( "activity" );
  for (size_t i=0; i<computed_track_records.size(); ++i)
  {
    track_record_type& r = computed_track_records[i];
    const track_handle_list_type& r_tracks = r.tracks();
    track_handle_list_type new_tracks;
    for (size_t j=0; j<r_tracks.size(); ++j)
    {
      track_handle_type t = r_tracks[j];
      if ( ! comms.query_id.exists( t.row ))
      {
        new_tracks.push_back( t );
        continue;
      }

      string qid = comms( t ).query_id();
      map< string, vector< size_t > >::const_iterator probe
        = truth_src.qid2activity_map.find( qid );
      if ( probe == truth_src.qid2activity_map.end() )
      {
        ostringstream oss;
        oss << "comms / scenario mismatch?  Comms query " << qid << " not in scenario?";
        throw runtime_error( oss.str() );
      }
      const vector< size_t >& activity_indices = probe->second;
      if (activity_indices.empty())
      {
        ostringstream oss;
        oss << "comms / scenario mismatch?  Comms query " << qid << " doesn't define any activities?";
        throw runtime_error( oss.str() );
      }
      // just add the first activity to the original track
      activity( t.row ) = activity_indices[0];
      new_tracks.push_back( t );

      // make copies for the other activities
      for (size_t k=1; k<activity_indices.size(); ++k)
      {
        // This is where a generic copy-the-track method would come in handy.
        // But would that be at the track_base_impl level?  Or at the element_descriptor level?
        track_handle_type new_t = comms_copy.create();
        comms_copy( new_t ).track_source() = comms( t ).track_source();
        comms_copy( new_t ).probability() = comms( t ).probability();
        comms_copy( new_t ).query_id() = comms( t ).query_id();

        frame_handle_list_type frames = track_oracle_core::get_frames( t );
        for (size_t f = 0; f<frames.size(); ++f)
        {
          const frame_handle_type& src_f = frames[f];
          frame_handle_type new_f = comms_copy( new_t ).create_frame();
          comms_copy[ new_f ].bounding_box() = comms[ src_f ].bounding_box();
          comms_copy[ new_f ].timestamp() = comms[ src_f ].timestamp();
        }

        // the next activity
        activity( new_t.row ) = activity_indices[k];

        new_tracks.push_back( new_t );
      }
    } // for all tracks

    LOG_INFO( main_logger, "Track record " << i << ": " << r_tracks.size() << " in; " << new_tracks.size() << " out" );
    r.set_tracks( new_tracks );

  } // for all track records
}


bool
parse_mgrs_ll_fields( const string& s,
                      string& lon_field,
                      string& lat_field )
{
  vul_reg_exp re_ll( "([^:]+):([^:]+)" );
  if ( ! re_ll.find( s ))
  {
    LOG_ERROR ( main_logger,"Couldn't parse longitude / latitude field names from '" << s << "'" );
    return false;
  }
  lon_field = re_ll.match( 1 );
  lat_field = re_ll.match( 2 );
  return true;
}

bool
parse_min_track_length_parameter( const string& s,
                                  size_t& min_truth_length,
                                  size_t& min_computed_length )
{
  vul_reg_exp re_tl( "([0-9]+):([0-9]+)" );
  if (! re_tl.find( s ))
  {
    LOG_ERROR( main_logger, "Couldn't parse minimum track lengths from '" << s << "'" );
    return false;
  }
  min_truth_length = boost::lexical_cast<size_t>( re_tl.match(1) );
  min_computed_length = boost::lexical_cast<size_t>( re_tl.match(2) );
  return true;
}


bool
track_record_type
::timestamp_overlaps( const track_record_type& other ) const
{
  const track_timestamp_stats_type& my_stats = this->stats();
  const track_timestamp_stats_type& other_stats = other.stats();

  // empty stats never overlap
  if ( my_stats.is_empty || other_stats.is_empty) return false;

  if ( my_stats.minmax_ts.first > other_stats.minmax_ts.second ) return false;
  if ( my_stats.minmax_ts.second < other_stats.minmax_ts.first ) return false;
  return true;
}

bool
check_vectors_for_timestamp_overlap( vector< track_record_type >& records )
{
  // need at least two to overlap
  if ( records.size() < 2) return false;

  for (unsigned i=0; i<records.size()-1; ++i)
  {
    const track_record_type& r = records[i];
    for (unsigned j=i+1; j<records.size(); ++j)
    {
      if ( r.timestamp_overlaps( records[j] ))
      {
        LOG_INFO( main_logger, "Timestamp overlap: " << i << " vs " << j << "\n"
                  << r.src_fn() << ": " << r.stats() << "\n"
                  << records[j].src_fn() << ": " << records[j].stats() );
        return true;
      }
    }
  }
  return false;
}

void
check_for_zero_area_boxes( const vector< track_record_type >& records )
{
  size_t warn_count = 0;
  scorable_track_type stt;
  for (size_t i=0; i<records.size(); ++i)
  {
    const track_record_type& r = records[i];
    const track_handle_list_type& r_tracks = r.tracks();
    for (size_t j=0; j<r_tracks.size(); ++j)
    {
      frame_handle_list_type frames = track_oracle_core::get_frames( r_tracks[j] );
      for (size_t k=0; k<frames.size(); ++k)
      {
        if ( vgl_area( stt[ frames[k] ].bounding_box() ) <= 0)
        {
          if (warn_count < 5 )
          {
            LOG_WARN( main_logger, "Zero-area-box: file " << r.src_fn()
                      << " track " << stt( r_tracks[j] ).external_id()
                      << " frame " << stt[ frames[k] ].timestamp_frame()
                      << " box " << stt[ frames[k] ].bounding_box() );
          }
          if (warn_count == 5)
          {
            LOG_WARN( main_logger, "(Further zero-area-box warnings suppressed)" );
          }
          ++warn_count;
        }
      }
    }
  }
  if (warn_count > 5)
  {
    LOG_WARN( main_logger,"Total of " << warn_count << " zero-area-boxes found" );
  }
}

void
check_for_increasing_frame_number_and_timestamps( const vector< track_record_type >& records )
{
  scorable_track_type stt;
  for (size_t i=0; i<records.size(); ++i)
  {
    const track_record_type& r = records[i];
    const track_handle_list_type& r_tracks = r.tracks();
    for (size_t j=0; j<r_tracks.size(); ++j)
    {
      frame_handle_list_type frames = track_oracle_core::get_frames( r_tracks[j] );
      if (frames.size() < 2) continue;

      unsigned last_frame_num = stt[ frames[0] ].timestamp_frame();
      ts_type last_ts = stt[ frames[0] ].timestamp_usecs();
      for (size_t k=1; k<frames.size(); ++k)
      {
        if ( ! stt.timestamp_frame.exists( frames[k].row )) continue;
        if ( ! stt.timestamp_usecs.exists( frames[k].row )) continue;
        unsigned this_frame_num = stt[ frames[k] ].timestamp_frame();
        ts_type this_ts = stt[ frames[k]] .timestamp_usecs();
        bool fn_increases = (last_frame_num < this_frame_num);
        bool ts_increases = (last_ts < this_ts);
        // sometimes the files are written in last-to-first order;
        // we just want to make sure that if the frame number increases,
        // the timestamp increases, and vice versa
        bool fn_and_ts_consistent = (fn_increases == ts_increases);
        if ( ! fn_and_ts_consistent )
        {
          LOG_WARN( main_logger, "Timebases heading in different directions: file " << r.src_fn()
                    << " track " << stt( r_tracks[j] ).external_id()
                    << " last frame / ts " << last_frame_num << " / " << last_ts
                    << " this frame / ts " << this_frame_num << " / " << this_ts );
        }
        last_frame_num = this_frame_num;
        last_ts = this_ts;
      }
    }
  }
}

track_timestamp_stats_type
track_set_minmax_timestamps( const vector< track_record_type >& records )
{
  track_timestamp_stats_type stats;
  for (size_t i=0; i<records.size(); ++i)
  {
    stats.combine_with_other( records[i].stats() );
  }
  return stats;
}

pair< size_t, size_t >
filter_on_timestamp_window( const time_window_filter& twf,
                            vector< track_record_type >& records )
{
  if ( ! twf.is_valid() )
  {
    throw runtime_error( "Filter-on-timestamp-window called with invalid time window filter" );
  }
  size_t in_c(0), out_c(0);

  for (size_t i=0; i<records.size(); ++i)
  {
    track_record_type& r = records[i];
    const track_handle_list_type& r_tracks = r.tracks();

    in_c += r_tracks.size();
    track_handle_list_type out;
    for (size_t j=0; j<r_tracks.size(); ++j)
    {
      if ( twf.track_passes_filter( r_tracks[j] ))
      {
        out.push_back( r_tracks[j] );
      }
    }
    out_c += out.size();
    r.set_tracks( out );
  }

  return make_pair( in_c, out_c );
}


vector< track_record_type >
load_tracks_from_file( const input_source_type& src )
{
  vector< track_record_type > ret;

  for (unsigned i=0; i<src.fn_list.size(); ++i)
  {
    track_record_type r;
    r.set_src_fn( src.fn_list[i] );
    LOG_INFO( main_logger, "About to load file " << i+1 << " of " << src.fn_list.size() << " : " << r.src_fn() << "...");
    track_handle_list_type input_tracks;
    if ( ! file_format_manager::read( r.src_fn(), input_tracks ))
    {
      LOG_ERROR( main_logger, "Couldn't load tracks from '" << r.src_fn() << "'");
      ret.clear();
      return ret;
    }
    LOG_INFO( main_logger, "read " << input_tracks.size() << " tracks");
    // only keep tracks longer than the requested number of states
    if ( src.min_track_length > 0 )
    {
      track_handle_list_type filtered;
      for (size_t j=0; j<input_tracks.size(); ++j)
      {
        size_t n = track_oracle_core::get_n_frames( input_tracks[j] );
        if (n >= src.min_track_length )
        {
          filtered.push_back( input_tracks[j] );
        }
      }
      LOG_INFO( main_logger, "Track length filtering requested; kept " << filtered.size()
                << " tracks with length >= " << src.min_track_length );
      input_tracks = filtered;
    }
    r.set_tracks( input_tracks );
    ret.push_back( r );
  }
  return ret;
}

bool
timestamp_paired_gtct( vector< track_record_type >& truth_track_records,
                       double truth_fps,
                       vector< track_record_type >& computed_track_records,
                       double computed_fps )
{
  if ( truth_track_records.size() != computed_track_records.size() )
  {
    LOG_ERROR( main_logger, "ERROR: paired gt/ct requested; loaded " << truth_track_records.size()
             << " truth track files, but " << computed_track_records.size()
             << " computed track files; should be equal");
    return false;
  }

  ts_type ts_offset = 0;
  scorable_track_type trk;
  const ts_type two_second_gap = static_cast<ts_type>(2.0e6);  // in usecs

  for (unsigned i=0; i<truth_track_records.size(); ++i)
  {
    // skip pathological cases
    if (truth_track_records[i].tracks().empty() && computed_track_records[i].tracks().empty())
    {
      continue;
    }

    // not refs because we recompute at the end of the loop
    track_timestamp_stats_type truth_stats = truth_track_records[i].stats();
    track_timestamp_stats_type computed_stats = computed_track_records[i].stats();
    track_timestamp_stats_type combined_stats( truth_track_records[i].stats() );
    combined_stats.combine_with_other( computed_stats );
    LOG_INFO( main_logger, "Processing gt/ct pair " << i << " : starting at:\n"
              << "truth: " << truth_stats.minmax_ts.first << " to " << truth_stats.minmax_ts.second
              << " ( " << truth_stats.minmax_ts.second - truth_stats.minmax_ts.first << " )\n"
              << "computed: " << computed_stats.minmax_ts.first << " to " << computed_stats.minmax_ts.second
              << " ( " << computed_stats.minmax_ts.second - computed_stats.minmax_ts.first << " )\n"
              << "combined: " << combined_stats.minmax_ts.first << " to " << combined_stats.minmax_ts.second
              << " ( " << combined_stats.minmax_ts.second - combined_stats.minmax_ts.first << " )" );


    //
    // if we have computed tracks but haven't managed to get them timestamps, that's
    // a dealbreaker
    //

    bool computed_tracks_okay = computed_track_records[i].tracks().empty() || computed_stats.has_timestamps;
    if ( ! computed_tracks_okay )
    {
      LOG_ERROR( main_logger, "Processing gt/ct pair " << i << ": computed tracks exist, but have no timestamps?" );
      return false;
    }

    //
    // If truth tracks don't have timestamps yet, we need to interpolate them in
    // so we can rebase them
    //

    if ( ! truth_stats.has_timestamps )
    {
      timestamp_generator tg =
        timestamp_generator_factory::from_tts( computed_stats, computed_fps,  truth_fps );
      tg.set_timestamps( truth_track_records[i].tracks() );
      truth_track_records[i].recompute_stats();
      truth_stats = truth_track_records[i].stats();
      combined_stats.reset();
      combined_stats.combine_with_other( truth_stats );
      combined_stats.combine_with_other( computed_stats );
    }

    LOG_INFO( main_logger, "Processing gt/ct pair " << i << " : rebased at:\n"
              << "truth: " << truth_stats.minmax_ts.first << " to " << truth_stats.minmax_ts.second
              << " ( " << truth_stats.minmax_ts.second - truth_stats.minmax_ts.first << " )\n"
              << "computed: " << computed_stats.minmax_ts.first << " to " << computed_stats.minmax_ts.second
              << " ( " << computed_stats.minmax_ts.second - computed_stats.minmax_ts.first << " )\n"
              << "combined: " << combined_stats.minmax_ts.first << " to " << combined_stats.minmax_ts.second
              << " ( " << combined_stats.minmax_ts.second - combined_stats.minmax_ts.first << " )" );


    vector< track_handle_list_type > tr_list;
    tr_list.push_back( truth_track_records[i].tracks() );
    tr_list.push_back( computed_track_records[i].tracks() );

    ts_type min_ts = combined_stats.minmax_ts.first;

    LOG_INFO( main_logger, "Processing gt/ct pair " << i << " : offset " << ts_offset
              << "; min ts " << min_ts );

    ts_type max_ts = 0;
    for (unsigned tr = 0; tr<tr_list.size(); ++tr )
    {
      const track_handle_list_type& tl = tr_list[ tr ];
      for (unsigned j=0; j<tl.size(); ++j)
      {
        frame_handle_list_type f = track_oracle_core::get_frames( tl[j] );
        for (unsigned k=0; k<f.size(); ++k)
        {
          pair< bool, ts_type > ts_probe = trk.timestamp_usecs.get( f[k].row );
          if ( ! ts_probe.first )
          {
            LOG_ERROR( main_logger, "Processing gt/ct pair " << i << " : frame without timestamp?" );
            return false;
          }
          ts_type this_ts = ts_probe.second;
          this_ts -= min_ts;
          this_ts += ts_offset;
          trk[ f[k] ].timestamp_usecs() = this_ts;
          if ( this_ts > max_ts ) max_ts = this_ts;
        }
      }
    }

    ts_offset = max_ts + two_second_gap;  // space out the tracks between sets

    LOG_INFO( main_logger, "Processing gt/ct pair " << i << " : before: "
              << combined_stats.minmax_ts.first << " to " << combined_stats.minmax_ts.second
              << " ( " << combined_stats.minmax_ts.second - combined_stats.minmax_ts.first << " )" );

    combined_stats.reset();
    for ( const auto& t: tr_list )
    {
      combined_stats.set_from_tracks( t );
    }

    LOG_INFO( main_logger, "Processing gt/ct pair " << i << " : during: "
              << combined_stats.minmax_ts.first << " to " << combined_stats.minmax_ts.second
              << " ( " << combined_stats.minmax_ts.second - combined_stats.minmax_ts.first << " )" );

    // recompute min/max timestamps in track records
    truth_track_records[i].recompute_stats();
    computed_track_records[i].recompute_stats();

    combined_stats.reset();
    for ( const auto& t: tr_list )
    {
      combined_stats.set_from_tracks( t );
    }

    LOG_INFO( main_logger, "Processing gt/ct pair " << i << " : after: "
              << combined_stats.minmax_ts.first << " to " << combined_stats.minmax_ts.second
              << " ( " << combined_stats.minmax_ts.second - combined_stats.minmax_ts.first << " )" );


  } //... for each truth track
  return true;
}

track_handle_list_type
decompose_track_into_frames( const track_handle_type& t )
{
  // break the track into single-frame tracks for scoring detections.
  // Note that ALL non-system fields are cloned; a track with ID 314
  // and 20 frames will result in 20 additional one-frame tracks, all with
  // ID 314.

  track_base_impl tbi;

  track_handle_list_type ret;

  frame_handle_list_type frames = track_oracle_core::get_frames( t );
  if (frames.size() == 1)
  {
    ret.push_back( t );
  }
  else
  {
    for (size_t i=0; i<frames.size(); ++i)
    {
      track_handle_type new_t = tbi.create();
      bool all_okay = track_oracle_core::clone_nonsystem_fields( t, new_t );
      if (all_okay)
      {
        frame_handle_type new_f = tbi.create_frame();
        all_okay = track_oracle_core::clone_nonsystem_fields( frames[i], new_f );
        if (all_okay)
        {
          ret.push_back( new_t );
        }
      }
    }
  }

  return ret;
}



bool
input_args_type
::process( track_handle_list_type& computed_tracks, track_handle_list_type& truth_tracks )
{
  if (this->time_window() == "help")
  {
    LOG_INFO( main_logger, string( "\n" ) <<
              time_window_filter::help_text() <<
              "\n" <<
              time_window_filter_factory::help_text() );
    return false;
  }

  if ( (this->computed_path() != "") && ( ! vul_file::is_directory( this->computed_path() )))
  {
    LOG_ERROR( main_logger, this->computed_path.option() << " is set to '" << this->computed_path()
               << "', but this directory does not exist" );
    return false;
  }

  if ( (this->truth_path() != "") && ( ! vul_file::is_directory( this->truth_path() )))
  {
    LOG_ERROR( main_logger, this->truth_path.option() << " is set to '" << this->truth_path()
               << "', but this directory does not exist" );
    return false;
  }

  // Even though we don't use the time window filter for a while, create it here
  // to find parsing errors quickly.  We can catch parsing errors of explicit
  // timestamps here, but we can't catch the 'special' time window codes until after
  // we've loaded the tracks.

  time_window_filter twf;
  if (this->time_window.set())
  {
    bool twf_special = time_window_filter_factory::code_is_special( this->time_window() );
    if (( ! twf_special ) && (! twf.set_from_string( this->time_window() )))
    {
      return false;
    }
  }

  //
  // Haven't ported over time window filters with paired-gtct logic yet
  //

  if ( this->time_window.set() && this->paired_gtct() )
  {
    LOG_ERROR( main_logger, "Can't use time windows with paired-gtct yet" );
    return false;
  }

  if ( ! (this->computed_tracks_fn.set() && this->truth_tracks_fn.set() ))
  {
    LOG_INFO( main_logger, "Must set both " << this->computed_tracks_fn.option() << " and "
             << this->truth_tracks_fn.option() << "");
    return false;
  }

  size_t min_truth_length, min_computed_length;
  if ( ! parse_min_track_length_parameter( this->track_length_filter(), min_truth_length, min_computed_length ))
  {
    LOG_INFO( main_logger, "parse min_track_length_parameter failed" );
    return false;
  }

  //
  // Create input sources from computed and truth tracks args
  //

  input_source_type computed_src( this->computed_tracks_fn(), this->computed_path, this->computed_fps(), min_computed_length );
  input_source_type truth_src( this->truth_tracks_fn(), this->truth_path, this->truth_fps(), min_truth_length );

  // verify we have plausible data in the input sources
  if (computed_src.fn_list.empty())
  {
    LOG_ERROR( main_logger, "Couldn't deduce an input source from '" << this->computed_tracks_fn() << "'");
    return false;
  }
  if (truth_src.fn_list.empty())
  {
    LOG_ERROR( main_logger, "Couldn't deduce an input source from '" << this->truth_tracks_fn() << "'");
    return false;
  }

  //
  // set the track format options
  //
  if (this->track_style_filter.set() )
  {
    kwxml_reader_opts& kwxml_opts = dynamic_cast<kwxml_reader_opts&>( file_format_manager::default_options( kwiver::track_oracle::TF_KWXML ) );
    kwxml_opts.set_track_style_filter( this->track_style_filter() );
  }

#ifdef SHAPELIB_ENABLED
  if ( ! this->apix_debug_fn().empty() )
  {
    apix_reader_opts& apix_opts = dynamic_cast<apix_reader_opts&>( file_format_manager::default_options( kwiver::track_oracle::TF_APIX ));
    apix_opts.set_verbose( true );
  }
#endif

  xgtf_reader_opts& xgtf_opts = dynamic_cast<xgtf_reader_opts&>( file_format_manager::default_options( kwiver::track_oracle::TF_XGTF ) );
  xgtf_opts.set_promote_pvmoving( this->promote_pvmoving() );
  comms_xml_reader_opts& comms_opts = dynamic_cast<comms_xml_reader_opts&>( file_format_manager::default_options( kwiver::track_oracle::TF_COMMS_XML));
  comms_opts.set_comms_qid( this->qid() );

  vector< track_record_type > truth_track_records = load_tracks_from_file( truth_src );
  // Truth tracks must be non-empty, else why are you trying to compute scores?
  if ( truth_track_records.empty() )
  {
    LOG_INFO( main_logger, "No truth tracks; score_tracks_loader returning false" );
    return false;
  }

  xgtf_opts.reset();
  comms_opts.reset();

  // if kw19 hack options are set, use them when reading computed tracks
  if (kw19_hack())
  {
    kwiver::track_oracle::kw18_reader_opts& kw18_opts =
      dynamic_cast< kwiver::track_oracle::kw18_reader_opts& >(
        file_format_manager::default_options( kwiver::track_oracle::TF_KW18 ));
    kw18_opts.set_kw19_hack( true );
  }

  vector< track_record_type > computed_track_records = load_tracks_from_file( computed_src );

  // if kw19, reset reader
  if (kw19_hack())
  {
    file_format_manager::default_options( kwiver::track_oracle::TF_KW18 );
  }

  // However, the computed tracks can be empty if, for example, the tracker missed everything.

  // Special VIRAT processing:
  // if truth_src is a scenario, and computed_src is a comms, then we should
  // have (activities, qids) in truth and (qids) in scenario, and can map over
  // activities to computed.
  set_virat_scenario_activities( truth_src, computed_track_records );

#ifdef KWANT_ENABLE_MGRS
  // set MGRS data, always for APIX tracks, optionally for everything else
  // ...for formats such as CSV, the user can specify which columns contain the
  // latitude / longitude data; parse that here
  string lon_field( "longitude" ), lat_field( "latitude" );
  parse_mgrs_ll_fields( this->mgrs_lon_lat_fields(), lon_field, lat_field );
  set_mgrs_data( truth_track_records, this->compute_mgrs_data, lon_field, lat_field );
  set_mgrs_data( computed_track_records, this->compute_mgrs_data, lon_field, lat_field );
#endif

  // special APIX processing: clear out old log if defined
  if ( ! this->apix_debug_fn().empty() )
  {
    LOG_INFO( main_logger, "Clearing APIX shapefile logs '" << this->apix_debug_fn() << "'" );
    vul_file::delete_file_glob( this->apix_debug_fn() );
  }

#ifdef SHAPELIB_ENABLED
  // special APIX procesing: set timestamps correctly
  convert_apix_timestamps( truth_track_records, this->apix_debug_fn() );
  convert_apix_timestamps( computed_track_records, this->apix_debug_fn() );
#endif

  // special VPD processing: copy object_id to external_id
  promote_object_id_to_external_id( truth_track_records );
  promote_object_id_to_external_id( computed_track_records );

  // special VPD processing: copy VPD event types to VIRAT event types
  promote_vpd_event_to_virat_event( truth_track_records );
  promote_vpd_event_to_virat_event( computed_track_records );


  // If frame-number-to-timestamp conversion is requested, do so here
  if ( this->ts_from_fn() )
  {
    promote_frame_number_to_timestamp( truth_track_records );
    promote_frame_number_to_timestamp( computed_track_records );
  }

  // Computed tracks *must* have timestamps.
  for (unsigned i=0; i<computed_track_records.size(); ++i)
  {
    track_record_type& cr = computed_track_records[i];

    if ( (! cr.tracks().empty() ) && (! cr.stats().all_have_timestamps ))
    {
      LOG_INFO( main_logger, "Computed tracks from '" << computed_track_records[i].src_fn()
               << "' don't seem to have timestamps?  Exiting");
      return false;
    }
  }

  // if pairing of gt / ct files was requested, process that now.
  if ( this->paired_gtct() )
  {
    if (! timestamp_paired_gtct( truth_track_records, this->truth_fps(), computed_track_records, this->computed_fps() ))
    {
      LOG_INFO( main_logger, "paired-gtct failed; score_tracks_loader returning false" );
      return false;
    }
  }

  // If truth tracks do not have timestamps, assign them.
  vector< unsigned > unstamped_truth_record_indices;
  for (unsigned i=0; i<truth_track_records.size(); ++i)
  {
    if ( (! truth_track_records[i].tracks().empty() ) &&
         (! truth_track_records[i].stats().all_have_timestamps ))
    {
      LOG_INFO( main_logger, "Truth track record '" << truth_track_records[i].src_fn() << "' has unstamped tracks");
      unstamped_truth_record_indices.push_back( i );
    }
  }

  //
  // begin hairy logic for all score_tracks use cases
  //

  if ( ! unstamped_truth_record_indices.empty() )
  {
    //
    // If both a virat scenario file and timebase file are specified,
    // allow the timebase to replace the scenario, but warn.
    //

    if ( this->xgtf_timestamps_fn.set() )
    {
      if ( ! truth_src.timestamp_generator_map.empty() )
      {
        LOG_WARN( main_logger, "*\n* ground truth timestamps loaded from scenario,\n"
                 << "* but " << this->xgtf_timestamps_fn.option() << " is also set;\n"
                 << "* values in this file will replace those from the scenario.\n"
                 << "*");
      }
      pair< bool, timestamp_generator_map_type > p =
        timestamp_generator_factory::from_timebase_file( this->xgtf_timestamps_fn() );
      if ( ! p.first )
      {
        LOG_ERROR( main_logger, "couldn't load xgtf timestamp adjustments; score_tracks_loader returning false" );
        return false;
      }
      truth_src.timestamp_generator_map = p.second;
    }

    // If the generator map is STILL empty (but we have unstamped
    // truth records), then hope we only need one value, which can be
    // set via the xgtf_base_ts option.

    if ( truth_src.timestamp_generator_map.empty() )
    {
      // If we need to estimate truth track timestamps and there is no supplied
      // xgtf timestamp map, then we can only continue if there is exactly one
      // ground truth file.  (Technically, one unstamped ground truth file, but
      // mixing stamped and unstamped ground truth files is definitely confusing
      // and probably a sign of an ill-conceived experiment.)
      if ( truth_track_records.size() != 1 )
      {
        LOG_INFO( main_logger, "Found " << truth_track_records.size() << " truth track files, but no"
                 << " XGTF timestamps found either via scenario or given\n"
                 << "via " << this->xgtf_timestamps_fn.option() << " option.  Exiting");
        return false;
      }

      // We need to get the frame 0 timestamp for a single truth track.
      // If it's explicitly given, use that; otherwise, require that there is
      // only one computed track file and estimate the timestamp from that.

      timestamp_generator tg;
      if ( (! this->xgtf_base_ts.set()) || (this->xgtf_base_ts() == "probe"))
      {
        // this can only work if there is only one computed track file and one truth track file
        if ( ( computed_track_records.size() != 1) || (truth_track_records.size() != 1) )
        {
          LOG_INFO( main_logger, "Ground truth track timestamp probe requested more than one "
                   << "computed and/or ground truth track file given, which is ambiguous.\n"
                   << "Exiting");
          return false;
        }

        const track_timestamp_stats_type& stats = computed_track_records[0].stats();
        tg = timestamp_generator_factory::from_tts( stats, this->computed_fps(), this->truth_fps() );
        LOG_INFO( main_logger, "Estimated frame 0 timestamp @ " << this->computed_fps() << " fps: "
                  << tg.get_base_ts() );

        if ( this->xgtf_base_ts() == "probe" )
        {
          // if probe-only was requested, exit here.
          exit( EXIT_SUCCESS );
        }
      }
      else
      {
        istringstream iss( this->xgtf_base_ts() );
        ts_type base_ts;
        if ( ! ( iss >> base_ts ))
        {
          LOG_ERROR( main_logger, "Error: couldn't parse base timestamp from '" << xgtf_base_ts() << "'");
          return false;
        }
        tg = timestamp_generator( this->truth_fps(), base_ts );
      }

      // We now have a factory with single base_ts to associate with our single truth track.
      // Insert into the xgtf_ts_map using the truth track filename, since that's
      // what's used down below as the lookup key.  This way, if you run score_tracks
      // like this:  --computed-tracks foo.kw18 --truth-tracks monday.xgtf, the timestamp
      // derived from "foo" is looked up via "monday.xgtf", which we can claim is unambiguous
      // because we require that (in this codepath) only one each of the computed and truth
      // track arguments are supplied.

      truth_src.timestamp_generator_map[ vul_file::basename( truth_track_records[0].src_fn() ) ] = tg;
    } // .. if xgtf_ts_map *was* empty...

    //
    // Set the timestamps in the truth tracks via the map.
    //

    for (unsigned i=0; i<unstamped_truth_record_indices.size(); ++i)
    {
      unsigned index = unstamped_truth_record_indices[i];
      track_record_type& r = truth_track_records[index];
      string base_fn = vul_file::basename( r.src_fn() );
      timestamp_generator_cit probe = truth_src.timestamp_generator_map.find( base_fn );
      if ( probe == truth_src.timestamp_generator_map.end() )
      {
        LOG_ERROR( main_logger, "Couldn't find a timestamp generator for truth track base '"
                 << base_fn << "' in the timestamp map?");
        return false;
      }

      const timestamp_generator& tg = probe->second;
      LOG_INFO( main_logger, "Applying base timestamp " << tg.get_base_ts() << " to " << r.src_fn() << "");
      tg.set_timestamps( r.tracks() );
      r.recompute_stats();

      // Both a sanity check and also to set the min/max timestamps in the record
      if ( (! r.tracks().empty() ) && (! r.stats().all_have_timestamps ))
      {
        LOG_INFO( main_logger, "Even after hacking, not all frames have timestamps?");
        return false;
      }
    }

  } // ...if unstamped_truth_record_indices.empty()

  //
  // end hairy logic
  //


  // if detection mode is set, break up the tracks into single frame tracks.
  if (this->detection_mode())
  {
    for (size_t i=0; i<truth_track_records.size(); ++i)
    {
      track_record_type& r = truth_track_records[i];
      track_handle_list_type tlist = r.tracks();
      track_handle_list_type n;
      for (size_t j=0; j<tlist.size(); ++j)
      {
        track_handle_list_type d = decompose_track_into_frames( tlist[j] );
        n.insert( n.end(), d.begin(), d.end() );
      }
      LOG_INFO(main_logger, "Detection mode: truth tracks " << r.src_fn() << " from " << tlist.size() <<
                " tracks to " << n.size() << " detections" );
      r.set_tracks( n );
    }
    track_field< double > relevancy( "relevancy" );
    for (size_t i=0; i<computed_track_records.size(); ++i)
    {
      track_record_type& r = computed_track_records[i];
      track_handle_list_type tlist = r.tracks();
      track_handle_list_type n;
      for (size_t j=0; j<tlist.size(); ++j)
      {
        track_handle_list_type d = decompose_track_into_frames( tlist[j] );

        // if kw19 hack is set, then the relevancy is set on the frames, not the tracks.
        // copy up to tracks.

        if (kw19_hack())
        {
          for (const auto& t: d)
          {
            auto frame_list = track_oracle_core::get_frames( t );
            // should only be one
            for (const auto& f: frame_list )
            {
              if (relevancy.exists( f.row ))
              {
                auto r = relevancy( f.row );
                relevancy( t.row ) = r;
              }
            }
          }
        }

        n.insert( n.end(), d.begin(), d.end() );
      }
      LOG_INFO(main_logger, "Detection mode: computed tracks " << r.src_fn() << " from " << tlist.size() <<
                " tracks to " << n.size() << " detections" );
      r.set_tracks( n );
    }
  }

  // if time window filtering has been requested, perform it here
  if (this->time_window.set())
  {
    if ( time_window_filter_factory::code_is_special( this->time_window() ))
    {
      track_timestamp_stats_type tstats = track_set_minmax_timestamps( truth_track_records );
      track_timestamp_stats_type cstats = track_set_minmax_timestamps( computed_track_records );
      twf = time_window_filter_factory::from_stats( this->time_window(), tstats, cstats );
      if (! twf.is_valid() ) return false;
    }
    // ... else, was parsed before we loaded the tracks above.

    if ( ! twf.is_valid() )
    {
      throw runtime_error( "time window filter '"+this->time_window()+"' is neither 'G', 'C', 'M', or otherwise valid" );
    }

    pair<size_t, size_t> c;
    c = filter_on_timestamp_window( twf, truth_track_records );
    LOG_INFO( main_logger, "Time window filtering on truth tracks: " << c.first << " in, " << c.second << " out" );
    c = filter_on_timestamp_window( twf, computed_track_records );
    LOG_INFO( main_logger, "Time window filtering on computed tracks: " << c.first << " in, " << c.second << " out" );
  }

  // Ground-truth tracks should not have overlapping timestamps.  We
  // can't assume that different track files with overlapping
  // timestamps have the same image coordinate system, and since our
  // overlaps are calculated in image coordinates, we'd run the risk
  // of scoring (say) computed tracks from one file against ground
  // truth tracks from another file if their timestamp domains were
  // not unique.
  //
  // ...turns out, some of the scenario truth files overlap by
  // e.g. nine seconds.  >sigh<

  if ( check_vectors_for_timestamp_overlap( truth_track_records ))
  {
    LOG_INFO( main_logger, "**\n** Some of the GROUND TRUTH tracks have overlapping timestamps.\n"
             << "** This may result in unreliable scores in the overlap area.\n"
             << "**");
  }

  // However, computed tracks may come from different sub-trackers;
  // their timestamps may overlap, in which case we'll issue a warning
  // and let the user decide if it's a problem.

  if ( check_vectors_for_timestamp_overlap( computed_track_records ))
  {
    LOG_INFO( main_logger, "**\n** Some of the computed tracks have overlapping timestamps.\n"
             << "** This may or may not make sense depending on your original source.\n"
             << "**");
  }


  // Some rudimentary sanity checking: ensure box areas are non-zero.
  check_for_zero_area_boxes( truth_track_records );
  check_for_zero_area_boxes( computed_track_records );
  // Check that the timestamps and frame numbers actually monotonically increase.
  check_for_increasing_frame_number_and_timestamps( truth_track_records );
  check_for_increasing_frame_number_and_timestamps( computed_track_records );


  // All set; copy the track handles to the output parameters
  for (unsigned i=0; i<truth_track_records.size(); ++i)
  {
    truth_tracks.insert( truth_tracks.end(),
                         truth_track_records[i].tracks().begin(),
                         truth_track_records[i].tracks().end() );
  }
  for (unsigned i=0; i<computed_track_records.size(); ++i)
  {
    computed_tracks.insert( computed_tracks.end(),
                            computed_track_records[i].tracks().begin(),
                            computed_track_records[i].tracks().end() );
  }

  // mark all the tracks as "loaded"
  track_field< kwiver::track_oracle::dt::utility::state_flags > state_flags;
  for (size_t i=0; i<truth_tracks.size(); ++i)
  {
    const frame_handle_list_type& frames = track_oracle_core::get_frames( truth_tracks[i] );
    for (size_t j=0; j<frames.size(); ++j)
    {
      state_flags( frames[j].row ).set_flag( "ATTR_SCORING_SRC_IS_TRUTH" );
    }
  }

  for (size_t i=0; i<computed_tracks.size(); ++i)
  {
    const frame_handle_list_type& frames = track_oracle_core::get_frames( computed_tracks[i] );
    for (size_t j=0; j<frames.size(); ++j)
    {
      state_flags( frames[j].row ).set_flag( "ATTR_SCORING_SRC_IS_COMPUTED" );
    }
  }

  // all done
  return true;
}

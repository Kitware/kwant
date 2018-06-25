/*ckwg +5
 * Copyright 2010-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <limits>

#include <vul/vul_arg.h>
#include <vul/vul_file.h>

#include <vgl/vgl_area.h>

#include <track_oracle/core/track_oracle_core.h>
#include <track_oracle/core/track_base.h>
#include <track_oracle/core/track_field.h>

#include <track_oracle/file_formats/file_format_manager.h>

#include <track_oracle/aries_interface/aries_interface.h>

#include <scoring_framework/score_tracks_hadwav.h>

#include <scoring_framework/matching_args_type.h>
#include <scoring_framework/score_tracks_loader.h>
#include <scoring_framework/timestamp_utilities.h>

#include <vital/config/config_block.h>
#include <json.h>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::cout;
using std::endl;
using std::make_pair;
using std::map;
using std::ofstream;
using std::ostream;
using std::ostringstream;
using std::pair;
using std::runtime_error;
using std::string;
using std::vector;

using kwiver::track_oracle::aries_interface;
using kwiver::track_oracle::frame_handle_list_type;
using kwiver::track_oracle::frame_handle_type;
using kwiver::track_oracle::track_field;
using kwiver::track_oracle::track_handle_list_type;
using kwiver::track_oracle::track_handle_type;
using kwiver::track_oracle::track_oracle_core;
using kwiver::track_oracle::file_format_enum;
using kwiver::track_oracle::file_format_manager;

using namespace kwiver::kwant;

struct output_args_type
{
  vul_arg< string > track_stats_fn;
  vul_arg< string > target_stats_fn;
  vul_arg< string > json_dump_fn;
  vul_arg< string > matches_dump_fn;
  vul_arg< string > frame_level_matches_fn;

  output_args_type()
    : track_stats_fn(  "--track-stats", "write track purity / continuity to file" ),
      target_stats_fn( "--target-stats", "write target purity / continuity to file" ),
      json_dump_fn(    "--j", "write results in json format to file" ),
      matches_dump_fn( "--matches", "write track match info to file" ),
      frame_level_matches_fn( "--frame-level-match","writes out gt to cp frame level match information")
  {}
};


struct normalization_args_type
{
  vul_arg<unsigned>   dep_num_frames; // deprecated!  Throw an error if set
  vul_arg<double>     dep_norm_area;  // deprecated!  Throw an error if set
  vul_arg<double>     norm_to_time;   // number of seconds in normalized output
  vul_arg<double>     norm_to_area;   // number of m^2 in normalized output
  vul_arg<double>     norm_data_time; // number of seconds in dataset
  vul_arg<double>     gsd;            // meters per pixel
  normalization_args_type()
    : dep_num_frames( "--num-frames", "DEPRECATED: Use --norm-data-time instead" ),
      dep_norm_area( "--norm-area", "DEPRECATED: Use --norm-to-area instead" ),
      norm_to_time( "--norm-to-time", "seconds in time units of normalized FAR (e.g. 3600 is per-hour)", 60.0 ),
      norm_to_area( "--norm-to-area", "meters^2 in area units of normalized FAR (e.g. 1.0e6 is km^2)", 1.0e6 ),
      norm_data_time( "--norm-data-time", "time span (in seconds) of input data for normalization (optional for radial overlap)" ),
      gsd( "--gsd", "ground sample distance, in meters/pixel, for normalizing image-overlap data" )
  {}
  void sanity_check();
};

void
normalization_args_type
::sanity_check()
{
  if ( this->dep_num_frames.set() )
  {
    ostringstream oss;
    oss << "The " << this->dep_num_frames.option() << " option is no longer used.\n"
        << "Please use the " << this->norm_data_time.option() << " option instead.\n";
    throw runtime_error( oss.str().c_str() );
  }
  if ( this->dep_norm_area.set() )
  {
    ostringstream oss;
    oss << "The " << this->dep_norm_area.option() << " option is no longer used.\n"
        << "Please use the " << this->norm_to_area.option() << " option instead.\n";
    throw runtime_error( oss.str().c_str() );
  }
}

void
write_stats( const map< track_handle_type, per_track_phase3_hadwav >& stats,
             const string& fn )
{
  track_field< unsigned > external_id( "external_id" );
  ofstream os( fn.c_str() );
  if ( ! os )
  {
    LOG_ERROR( main_logger, "Couldn't open '" << fn << "' for writing?");
    return;
  }
  for (map< track_handle_type, per_track_phase3_hadwav >::const_iterator i = stats.begin();
       i != stats.end();
       ++i )
  {
    if (external_id.exists( i->first.row ) )
    {
      os << external_id( i->first.row ) << " ";
    }
    else
    {
      os << "-1 ";
    }
    os << i->second.purity << " " << i->second.continuity << " "
       << i->second.dominant_track_id << " " << i->second.dominant_track_size << " "
       << i->second.dominated_track_lifetime << "\n";
  }
}

void
write_per_activity( ostream& os,
                    const track_handle_list_type& gt_list,
                    const track_handle_list_type& ct_list,
                    const track2track_phase1 p1 )
{
  // key: activity index;
  // val.first: count of gt tracks with that activity;
  // val.second: count of gt tracks matched by any ct tracks
  map< int, pair< unsigned, unsigned > > act_to_gtct_map;
  typedef map< int, pair< unsigned, unsigned > >::const_iterator a2c_cit;

  track_field<int> activity_field( "activity" );
  for (unsigned i=0; i<gt_list.size(); ++i)
  {
    track_handle_type gt = gt_list[i];
    int act = activity_field( gt.row );

    pair<unsigned, unsigned>& gtct = act_to_gtct_map[ act ];
    ++gtct.first;

    bool was_matched = false;
    for (unsigned j=0; j<ct_list.size(); ++j)
    {
      track_handle_type ct = ct_list[ j ];
      track2track_type key = make_pair( gt, ct );
      if (p1.t2t.find( key ) != p1.t2t.end())
      {
        was_matched = true;
      }
    }
    if ( was_matched )
    {
      ++gtct.second;
    }

  }

  const map< size_t, string >& i2a = aries_interface::index_to_activity_map();
  const map< size_t, string >& i2pvo = aries_interface::index_to_PVO_map();
  map< size_t, string >::const_iterator probe;
  for (a2c_cit i = act_to_gtct_map.begin(); i != act_to_gtct_map.end(); ++i)
  {
    probe = i2a.find( i->first );
    if ( probe != i2a.end() )
    {
      os << probe->second;
    }
    else
    {
      os << "(no activity for " << i->first << "?)";
    }
    os << " ";
    probe = i2pvo.find( i->first );
    if ( probe != i2pvo.end() )
    {
      os << probe->second;
    }
    else
    {
      os << "(no PVO for " << i->first << "?)";
    }

    unsigned nTotalGT = i->second.first;
    unsigned nMatchedGT = i->second.second;
    double pd = 1.0 * nMatchedGT / nTotalGT;
    os << " total-gt: " << nTotalGT << "  matched-gt: " << nMatchedGT << " pd: " << pd << "\n";
  }
}

void
write_activity_overlay( ostream& os,
                        const track_handle_list_type& gt_list,
                        const track_handle_list_type& ct_list,
                        const track2track_phase1 p1 )
{
  // This is the framelist format for  overlay_score_tracks:
  // repeated blocks of:
  // activity-name number-gt-frames number-ct-frames  [one]
  // frame-num {"matched"|"unmatched"} ulx uly lrx lry  [one per gt/ct frame]
  //
  // ... the implication is that unmatched gt frames will have 0 ct frames.
  // We're keying only on gt frames, so we'll never have 0 gt frames but non-zero ct
  // frames (i.e. we don't write out false alarms.  We could, but we don't.)
  // Each gt-frame-num appears once, but a ct-frame-num may appear multiple times.
  //
  // However, we *can't* write out frames from computed tracks not
  // belonging to overlaps because we don't have their matching
  // frame-number / timestamp alignment (argh!)

  const map< size_t, string >& i2a = aries_interface::index_to_activity_map();
  track_field<int> activity_field( "activity" );
  scorable_track_type trk;

  for (unsigned i=0; i<gt_list.size(); ++i)
  {
    track_handle_type gt = gt_list[i];

    // now we want to collect the (possibly) empty set of computed tracks
    // which overlay gt, take all their boxes, and sort them into a map
    // of frame-number -> list-of( box, overlap ) .

    map< frame_handle_type, bool > frame_matches;  // track-oracle handle -> true-if-matched
    map< unsigned, vector< frame_handle_type> > ct_frame_boxes; // frame number -> list of frames

    unsigned nCT = 0;
    for (unsigned j=0; j<ct_list.size(); ++j)
    {
      track_handle_type ct = ct_list[j];

      // first, enter any matching frames into ct_frame_matches
      track2track_type key = make_pair( gt, ct );
      map< track2track_type, track2track_score >::const_iterator probe = p1.t2t.find( key );
      if ( probe != p1.t2t.end() )
      {
        const vector< track2track_frame_overlap_record >& m = probe->second.frame_overlaps;
        for (unsigned k=0; k<m.size(); ++k)
        {
          // Truth and computed frames match
          frame_matches[ m[k].truth_frame ] = true;
          frame_matches[ m[k].computed_frame ] = true;
          // record computed box in frame-number system of truth tracks
          ct_frame_boxes[ m[k].fL_frame_num ].push_back( m[k].computed_frame );
          ++nCT;
        }
      }
    }

    // That was all the positive frames.  Now we want those frames of the matching
    // ground-truth tracks which DIDN'T show up in the overlap records.  Those frames
    // will not exist in in the frame_matches map; we'll explicitly record those as match=false.

    frame_handle_list_type gt_frames = track_oracle_core::get_frames( gt );
    for (unsigned j=0; j<gt_frames.size(); ++j)
    {
      if ( frame_matches.find( gt_frames[j] ) == frame_matches.end() )
      {
        frame_matches[ gt_frames[j] ] = false;
      }
    }

    // we'd like to draw the un-matched computed frames, but we don't have their timestamps
    // or framenumbers in the gt-track frame sequence. :(  (Need to re-factor phase1.)

    // collect the activity name and the ground truth frames
    string activity = "undefined";
    map<size_t, string>::const_iterator i2a_cit = i2a.find( activity_field( gt.row ));
    if ( i2a_cit != i2a.end() )
    {
      activity = i2a_cit->second;
    }

    // now we can start writing out the framelist entry
    os << activity << " " << gt_frames.size() << " " << nCT << "\n";

    // write out the gt frames
    for (unsigned j=0; j<gt_frames.size(); ++j)
    {
      bool matched = frame_matches[ gt_frames[j] ];
      vgl_box_2d<double> box = trk[ gt_frames[j] ].bounding_box();
      os << trk[ gt_frames[j] ].timestamp_frame() << " ";
      if (matched) os << "matched "; else os << "unmatched ";
      os << box.min_x() << " " << box.min_y() << " " << box.max_x() << " " << box.max_y() << "\n";
    }
    // write out the ct frames
    map< unsigned, vector< frame_handle_type > >::const_iterator it;
    for (it = ct_frame_boxes.begin(); it != ct_frame_boxes.end(); ++it)
    {
      for (unsigned j=0; j<it->second.size(); ++j)
      {
        frame_handle_type t = it->second[j];
        bool matched = frame_matches[ t ];
        vgl_box_2d<double> box = trk[ t ].bounding_box();
        os << it->first << " ";
        if (matched) os << "matched "; else os << "unmatched ";
        os << box.min_x() << " " << box.min_y() << " " << box.max_x() << " " << box.max_y() << "\n";
      }
    }
  } // ...for each ground truth track

  // all done!
}

//
// This computes a normalization factor to convert the raw FA count to
// e.g. FA / km^2 / minute .
//
// The user can supply the desired output units via --norm-to-area
// (meters) and --norm-to-time (seconds); for example, norm-to-area =
// 1.0e6 is 1 km^2; norm-to-time = 60 is a minute, leading to km^2 / minutes.
//
//
// Mapping the input data to world-units of space and time requires knowing
// the spatial and temporal footprints of the data.  We can get this information
// in different ways, depending on the overlap method (radial or image-box).
//
// - Time footprint: --norm-data-time (in seconds).  This is the timespan
// of the dataset.  We could auto-detect this from the ground-truth
// and computed tracks, the same way the time_window_filter is set.
// In fact, for radial overlaps, we can deduce the this from the
// computed tracks, on the assumption that WAMI data is dense enough
// that the time window of the computed tracks is a good
// approximation.  I'm less certain that this would hold for FMV,
// hence, it's mandatory for datasets with image-coordinate overlaps
// (usually FMV datasets.)
//
// - Spatial footprint: In all cases, an AOI is required.  For radial
// overlap, the AOI is lat/lon, so we directly compute the spatial
// footprint.  For image overlap, the AOI is in pixels, so the gsd
// (meters per pixel) must be given.
//


pair< bool, double >
compute_normalization_factor( const phase1_parameters& p1_params,
                              const matching_args_type& matching_args,
                              normalization_args_type& normalization_args,   // non-const because option() isn't const
                              const track_handle_list_type& computed_tracks )
{
  //
  // normalization, part 1:
  //
  // Parse the AOI / norm-data-time / gsd options to get the normalization
  // factor in m^2/sec; set compute_norm to false if it doesn't work out
  //

  double norm = 1.0;
  bool compute_norm = true;
  if ( matching_args.radial_overlap() >= 0.0 )
  {
    // We're in geo-coords, so we can get the normalizing area straight from the AOI
    // if we don't have an AOI, we're out of luck for the moment until a patch
    // comes in to get the normalizing area some other way
    size_t n_mgrs_aoi = p1_params.mgrs_aoi_list.size();
    if ( n_mgrs_aoi == 0)
    {
      LOG_INFO( main_logger, "Normalization: geo-coords used, but no AOI set; normalization disabled" );
      compute_norm = false;
    }
    else
    {
      double norm_area = 0.0;
      double sum_areas = 0.0;
      for (size_t aoi_i = 0; aoi_i < n_mgrs_aoi; ++aoi_i)
      {
        double a = vgl_area( p1_params.mgrs_aoi_list[ aoi_i ].aoi ); // area in m^2
        LOG_INFO( main_logger, "Normalization: geo-coords AOI " << aoi_i << " area: " << a << " m^2" );
        sum_areas += a;
      }
      if ( n_mgrs_aoi == 1)
      {
        norm_area = sum_areas;
      }
      else
      {
        norm_area = sum_areas / n_mgrs_aoi;
        LOG_INFO( main_logger, "Normalization: average of geo-coords AOI area " << norm_area << " m^2" );
      }
      // assume that in a geo-coord (i.e. WAMI) situation, we can fairly get the
      // correct normalization time range from the min/max of the computed tracks
      // I'm pretty sure we compute these values several other places in the code... sigh

      if ( normalization_args.norm_data_time.set() )
      {
        LOG_INFO( main_logger, "Normalization: using user-specified normalization data time" );
      }
      else
      {
        timestamp_utilities::track_timestamp_stats_type tts( computed_tracks );
        if ( ! tts.has_timestamps )
        {
          LOG_WARN( main_logger, "Computed tracks do not have timestamps; not computing temporal FA normalization" );
          norm = 1.0;
          compute_norm = false;
        }
        else
        {
          double ts_span_seconds = ((tts.minmax_ts.second - tts.minmax_ts.first) / 1.0e6);
          LOG_INFO( main_logger, "Normalization: computed time extent " << (ts_span_seconds / 60) << " minutes" );
          norm = norm_area * ts_span_seconds;
        }
      }
    }
  } // ...radial overlap
  else
  {
    if ( p1_params.b_aoi.is_empty() )
    {
      LOG_INFO( main_logger, "Normalization: no AOI set; frame FAR normalization disabled");
      compute_norm = false;
    }
    if ( ! normalization_args.norm_data_time.set() )
    {
      LOG_INFO( main_logger, "Info: " << normalization_args.norm_data_time.option()
                << " not set; frame FAR normalization disabled");
      compute_norm = false;
    }

    if (compute_norm)
    {
      double g = normalization_args.gsd(); // meters-per-pixel
      double data_spatial_footprint = vgl_area( p1_params.b_aoi ) * g * g; // m^2
      norm = data_spatial_footprint * normalization_args.norm_data_time(); // (m^2)*s
    }
  }

  // End part 1.

  if (! compute_norm ) return make_pair( false, 1.0 );

  LOG_INFO( main_logger, "Normalization factor (raw): " << norm << " m^2 * s" );

  //
  // part 2: convert to requested units
  //

  norm = norm / ( normalization_args.norm_to_area() * normalization_args.norm_to_time() );
  LOG_INFO( main_logger, "Normalization factor (user units): " << norm << " tracks per ( "
            << normalization_args.norm_to_area() << " m^2) per ( "
            << normalization_args.norm_to_time() << " seconds )" );


  return make_pair( true, norm );
}

int main( int argc, char *argv[] )
{
  vul_arg<bool> score_hadwav_flag( "--hadwav", "Use hadwav scoring system", true);
  vul_arg<bool> verbose_flag( "-v", "dump more debugging information", false );
  vul_arg<bool> disable_sanity_checks_arg( "--disable-sanity-checks", "Check for basic track overlaps before scoring", false );
  vul_arg< string > t2t_dump_fn_arg( "--t2t-dump-file", "dump track-to-track details here" );
  vul_arg< bool > disable_t2t_dump_cmd_file( "--t2t-disable-cmd-file", "set to disable default creation of 'plot-{t2t-dump-file}-cmd.txt'" );
  vul_arg< string > activity_pd_dump_fn_arg( "--act-pd-file", "write per-activity track Pd here (if supported by ground-truth)" );
  vul_arg< string > activity_overlay_fn_arg( "--act-overlay-file", "write activity overlay data for overlay_score_tracks" );
  vul_arg< bool > display_git_hash( "--git-hash", "Display git hash and exit", false);
  vul_arg< string > track_dump_fn_arg( "--write-tracks", "Write annotated input tracks to this file (either .kwcsv or .kwiver)" );

  input_args_type input_args;
  output_args_type output_args;
  matching_args_type matching_args;
  normalization_args_type normalization_args;

  ostringstream arg_oss;
  for (int i=0; i<argc; ++i) arg_oss << argv[i] << " ";
  vul_arg_parse( argc, argv );

  //  LOG_INFO( main_logger, "GIT-HASH: " << VIDTK_GIT_VERSION );
  LOG_INFO( main_logger, "GIT-HASH: output not supported yet" );
  if (display_git_hash())
  {
    return EXIT_SUCCESS;
  }

  //
  // Toplevel sanity checks
  //

  normalization_args.sanity_check();

  if ( ( ! matching_args.sanity_check()) || ( ! matching_args.parse_min_frames_arg() ))
  {
    return EXIT_FAILURE;
  }

  if (! (score_hadwav_flag() ))
  {
    LOG_INFO( main_logger, "No scoring protocol specified; please choose at least one of --hadwav, --aipr, etc.");
    return EXIT_FAILURE;
  }

  // if the user requested annotated track output, make sure the format is unambiguous
  if ( track_dump_fn_arg.set() )
  {
    vector< file_format_enum > fmts = file_format_manager::globs_match( track_dump_fn_arg() );
    if ( fmts.size() != 1 )
    {
      LOG_ERROR( main_logger, "Track output filename matches " << fmts.size() << " formats; use either .kwcsv or .kwiver" );
      return EXIT_FAILURE;
    }
  }

  // if the user specified radial_overlap, set input_arg's compute_mgrs_data flag
  if ( matching_args.radial_overlap() >= 0.0 )
  {
    input_args.compute_mgrs_data = true;
  }

  //
  // deal with the inputs to get two sets of tracks, one computed,
  // one ground-truth.  Each will have timestamps.
  //

  // This is a good point to emit the command line for logging
  LOG_INFO( main_logger, "Command line:\n" << arg_oss.str() );

  track_handle_list_type computed_tracks, truth_tracks;
  if ( ! input_args.process( computed_tracks, truth_tracks ))
  {
    return EXIT_FAILURE;
  }

  //
  // if activity pd or overlays were requested, but we don't have
  // ground truth labels to support it, exit now rather than surprise
  // the user later
  //

  if ( ! truth_tracks.empty() )
  {
    track_field<int> activity_field( "activity" );
    bool has_activities = activity_field.exists( truth_tracks[0].row );
    if ( activity_pd_dump_fn_arg.set() && ( ! has_activities ))
    {
      LOG_INFO( main_logger, "Per-activity track pD requested, but ground-truth tracks don't have activities; exiting");
      return EXIT_FAILURE;
    }
    if ( activity_overlay_fn_arg.set() && ( ! has_activities ))
    {
      LOG_INFO( main_logger, "Activity overlays requested, but ground-truth tracks don't have activities; exiting");
      return EXIT_FAILURE;
    }
  }


  phase1_parameters p1_params;
  if ( disable_sanity_checks_arg() )
  {
    p1_params.perform_sanity_checks = false;
  }

  if ( ! p1_params.processMatchingArgs( matching_args ) )
  {
    return EXIT_FAILURE;
  }

  pair< bool, double > norm = compute_normalization_factor( p1_params, matching_args, normalization_args, computed_tracks );

  if( matching_args.bbox_expansion.set() )
  {
    double x = matching_args.bbox_expansion();
    p1_params.expand_bbox = true;
    double bbox_expansion_in_pixels = 0.0;
    if ( normalization_args.gsd.set() )
    {
      if (normalization_args.gsd() <= 0.0)
      {
        LOG_ERROR( main_logger, "Normalization GSD, if set, must be > 0; currently set to " << normalization_args.gsd() );
        return EXIT_FAILURE;
      }
      bbox_expansion_in_pixels = x / normalization_args.gsd();
      LOG_INFO( main_logger, "bbox expansion of " << x << " is " << bbox_expansion_in_pixels << " pixels" );
    }
    else
    {
      bbox_expansion_in_pixels = x;
      LOG_INFO ( main_logger,"bbox expansion set to " << x << " pixels" );
    }
    p1_params.bbox_expansion = bbox_expansion_in_pixels;
  }

  // filter the tracks on the AOI; also set frame_has_been_matched() state

  track_handle_list_type aoi_filtered_truth_tracks, aoi_filtered_computed_tracks;
  p1_params.filter_track_list_on_aoi( truth_tracks, aoi_filtered_truth_tracks );
  p1_params.filter_track_list_on_aoi( computed_tracks, aoi_filtered_computed_tracks );

  track2track_phase1 p1(p1_params);
  p1.compute_all( aoi_filtered_truth_tracks, aoi_filtered_computed_tracks );

  LOG_INFO( main_logger, "p1: AOI kept "
           << aoi_filtered_truth_tracks.size() << " of " << truth_tracks.size() << " truth tracks; "
           << aoi_filtered_computed_tracks.size() << " of " << computed_tracks.size() << " computed tracks");

  if ( t2t_dump_fn_arg.set() )
  {
    p1.debug_dump( aoi_filtered_truth_tracks,
                   aoi_filtered_computed_tracks,
                   t2t_dump_fn_arg(),
                   /* should be min timestamp = */ 0 );
    if ( ! disable_t2t_dump_cmd_file() )
    {
      string cmd_fn = "plot-"+t2t_dump_fn_arg()+"-cmd.txt";
      ofstream os( cmd_fn.c_str() );
      if ( ! os )
      {
        LOG_ERROR( main_logger, "Couldn't write '" << cmd_fn << "' (not fatal, but weird)");
      }
      else
      {
        os << "set title \"" << t2t_dump_fn_arg() << "\"\n";
        os << "set key bottom right\n";
        os << "plot "
           << "\"" << t2t_dump_fn_arg() << "-gt.dat\" ls 2 lw 2 w points t \"gt\", "
           << "\"" << t2t_dump_fn_arg() << "-m0.dat\" ls 1 w lines t \"matched\", "
           << "\"" << t2t_dump_fn_arg() << "-m1.dat\" w lines t \"unmatched\"\n";
        LOG_INFO( main_logger, "Info: wrote '" << cmd_fn << "'");
      }
    }
  }

  if ( activity_pd_dump_fn_arg.set() )
  {
    ofstream os( activity_pd_dump_fn_arg().c_str() );
    if ( ! os )
    {
      LOG_ERROR( main_logger, "Couldn't open '" << activity_pd_dump_fn_arg() << "' for writing; exiting");
      return EXIT_FAILURE;
    }
    write_per_activity( os, aoi_filtered_truth_tracks, aoi_filtered_computed_tracks, p1 );
  }

  if ( activity_overlay_fn_arg.set() )
  {
    ofstream os( activity_overlay_fn_arg().c_str() );
    if ( ! os )
    {
      LOG_ERROR( main_logger, "Couldn't open '" << activity_overlay_fn_arg() << "' for writing; exiting");
      return EXIT_FAILURE;
    }
    write_activity_overlay( os, aoi_filtered_truth_tracks, aoi_filtered_computed_tracks, p1 );
  }

  JSONNode json_root(JSON_NODE);  // for json output option

  if(score_hadwav_flag())
  {
    track2track_phase2_hadwav p2( verbose_flag() );
    p2.compute( aoi_filtered_truth_tracks, aoi_filtered_computed_tracks, p1 );

    LOG_INFO( main_logger, "p2...");

    if ( output_args.matches_dump_fn.set() )
    {
      ofstream os( output_args.matches_dump_fn().c_str() );
      if ( ! os )
      {
        LOG_ERROR( main_logger, "Couldn't dump match info to '" << output_args.matches_dump_fn() << "'?");
      }
      else
      {
        p2.debug_dump( os );
      }
    }

    if ( output_args.frame_level_matches_fn.set() )
    {
      ofstream os( output_args.frame_level_matches_fn().c_str() );
      if ( ! os )
      {
        LOG_ERROR( main_logger, "Couldn't dump match info to '" << output_args.frame_level_matches_fn() << "'?");
      }
      else
      {
        p1.debug_dump( os );
      }
    }

    overall_phase3_hadwav p3;
    p3.verbose = verbose_flag();
    p3.compute( p2 );

    if ( output_args.track_stats_fn.set() )
    {
      LOG_INFO( main_logger, "Writing " << output_args.track_stats_fn() << "...");
      write_stats( p3.get_mitre_track_stats(), output_args.track_stats_fn() );
    }
    if ( output_args.target_stats_fn.set() )
    {
      LOG_INFO( main_logger, "Writing " << output_args.target_stats_fn() << "...");
      write_stats( p3.get_mitre_target_stats(), output_args.target_stats_fn() );
    }

    cout << "HADWAV Scoring Results:" << endl;

    cout << "  Detection-Pd: " << p2.detectionPD << endl
             << "  Detection-FA: " << p2.detectionFalseAlarms << endl
             << "  Detection-PFA: " << p2.detectionPFalseAlarm << endl;

    if (norm.first)
    {
      cout << "  Frame-NFAR: " << p2.frameFA/norm.second << endl;
    }
    else
    {
      cout << "  Frame-NFAR: not computed" << endl;
    }

    cout << "  Track-Pd: " << p3.trackPd << endl;
    cout << "  Track-FA: " << p3.trackFA << "" << endl;

    double computed_track_pfa =
      aoi_filtered_computed_tracks.empty()
      ? 0.0
      : p3.trackFA / aoi_filtered_computed_tracks.size();
    cout << "  Computed-track-PFA: " << computed_track_pfa << endl;

    if (norm.first)
    {
      cout << "  Track-NFAR: " << p3.trackFA/norm.second << "" << endl;
    }
    else
    {
      cout << "  Track-NFAR: not computed" << endl;
    }
    cout << "  Avg track (continuity, purity ): " << p3.avg_track_continuity
             << ", " << p3.avg_track_purity << endl;
    cout << "  Avg target (continuity, purity ): " << p3.avg_target_continuity << ", "
             << p3.avg_target_purity << endl;
    cout <<  "  Track-frame-precision: " << p2.trackFramePrecision << endl;

    if ( output_args.json_dump_fn.set() )
    {
      // Add json objects
      JSONNode hadwav_node(JSON_NODE);
      hadwav_node.set_name("hadwav-results");

      hadwav_node.push_back(JSONNode("detection-pd", p2.detectionPD));
      hadwav_node.push_back(JSONNode("detection-false-alarms", p2.detectionFalseAlarms));
      hadwav_node.push_back(JSONNode("detection-probability-false-alarms", p2.detectionPFalseAlarm));
      hadwav_node.push_back(JSONNode("track-pd", p3.trackPd));
      hadwav_node.push_back(JSONNode("track-fa", p3.trackFA));
      hadwav_node.push_back(JSONNode("track-fp", p2.trackFramePrecision));
      if (norm.first)
      {
        hadwav_node.push_back(JSONNode("frame-nfar", p2.frameFA/norm.second));
        hadwav_node.push_back(JSONNode("track-nfar", p3.trackFA/norm.second));
      }
      else
      {
        hadwav_node.push_back(JSONNode("frame-nfar", "not computed"));
        hadwav_node.push_back(JSONNode("track-nfar", "not computed"));
      }
      hadwav_node.push_back(JSONNode("avg-track-continuity", p3.avg_track_continuity));
      hadwav_node.push_back(JSONNode("avg-track-purity", p3.avg_track_purity));
      hadwav_node.push_back(JSONNode("avg-target-continuity", p3.avg_target_continuity));
      hadwav_node.push_back(JSONNode("avg-target-purity", p3.avg_target_purity));

      json_root.push_back(hadwav_node);
    }
  }

  if ( output_args.json_dump_fn.set() )
  {
    // Add json objects
    JSONNode version_node(JSON_NODE);
    version_node.set_name("vidtk-meta");
    //    version_node.push_back(JSONNode("version", VIDTK_GIT_VERSION));
    version_node.push_back(JSONNode("version", "not-available" ));
    json_root.push_back(version_node);
  }

  if ( output_args.json_dump_fn.set() )
  {
    ofstream os( output_args.json_dump_fn().c_str() );
    if ( ! os )
    {
      LOG_ERROR( main_logger, "Couldn't dump results to '" << output_args.json_dump_fn() << "'?");
    }
    else
    {
      string json_output = json_root.write_formatted();
      os << json_output;
      os.close();
    }
  }

  if ( track_dump_fn_arg.set() )
  {
    track_handle_list_type all_tracks;
    all_tracks.insert( all_tracks.end(), aoi_filtered_truth_tracks.begin(), aoi_filtered_truth_tracks.end() );
    all_tracks.insert( all_tracks.end(), aoi_filtered_computed_tracks.begin(), aoi_filtered_computed_tracks.end() );
    bool rc = file_format_manager::write( track_dump_fn_arg(), all_tracks, kwiver::track_oracle::TF_INVALID_TYPE );
    LOG_INFO( main_logger, "Write returned " << rc );
  }
}

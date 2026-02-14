/*ckwg +5
 * Copyright 2011-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_SCORE_TRACKS_LOADER_H
#define INCL_SCORE_TRACKS_LOADER_H

/// There are some constraints that need to be satisfied when
/// loading tracks for scoring, mostly dealing with ensuring that
/// each frame has a timestamp.  This API provides a uniform set of
/// vul_args for the client to initialize and the ability to load
/// single tracks or sets of tracks, estimate timestamps from
/// computed tracks back to ground truth tracks, etc.
///
/// The client should create one instance of this type before calling
/// vul_arg_parse, and then call process() after calling vul_arg_parse,
/// and if process() returns true, then the ground-truth and computed track
/// lists are guaranteed to have timestamps.
///

#include <vital/vital_config.h>
#include <scoring_framework/score_tracks_loader_export.h>

#include <vul/vul_arg.h>

#include <track_oracle/core/track_oracle_core.h>
#include <track_oracle/core/track_base.h>
#include <track_oracle/core/track_field.h>

namespace kwiver {
namespace kwant {

struct SCORE_TRACKS_LOADER_EXPORT input_args_type
{
  // both of these must be set
  // each one of:
  // - a filename
  // - a string '@filelist', filelist is a file listing the track files
  // - a virat scenario file

  vul_arg< std::string > computed_tracks_fn;
  vul_arg< std::string > truth_tracks_fn;

  // if set, will override autodetection of file formats
  // 'help' to list valid formats

  vul_arg< std::string > computed_format;
  vul_arg< std::string > truth_format;

  // if set, will override path components in file when loading
  // Example: if --truth-path=/data/foo/bar, then:
  //
  // --truth-tracks sample.kw18  => /data/foo/bar/sample.kw18
  // --truth-tracks /tmp/sample.kw18 =>  /data/foo/bar/sample.kw18
  //
  // ...ditto for all filenames in a '@filelist' set or a scenario

  vul_arg< std::string > computed_path;
  vul_arg< std::string > truth_path;

  // XGTF does not have timestamps.  Since we align on timestamps,
  // when xgtf files are used, we need to add timestamps by starting
  // with a timestamp for frame 0 and linearly stamping subsequent frames
  // using the fps.

  vul_arg< double > computed_fps;
  vul_arg< double > truth_fps;

  // So where does the frame 0 come from?
  // (1) it may be explicitly listed in an xgtf_timestamps file
  // (2) it may be explicitly stated on the command line
  // (3) it may be estimated from the computed tracks
  // (4) it may be fabricated if we know a priori the XGTF aligns with the computed track
  //
  // (1) must be used when there are multiple XGTF files.
  //
  // (2) and (3) can only be used for single files.
  //
  // (4) is mutually exclusive from (1), (2), or (3).  If --paired-gtct is used,
  // then we require a one-to-one pairing between computed and ground truth,
  // and that this pairing is maintained in the order files are listed in the @filelist
  // files.  We assume each (gt, ct) pair is internally aligned on timestamps and
  // fabricate the timestamps by adding an offset to each timestamp in (gt, ct); this
  // offset is the highest timestamp in the previous pair plus some padding.  This will
  // only break any attempts to rederive timestamps based on estimating a t0 timestamp
  // based on the frame rate and a timestamp for a non-t0 frame, which is why (4)
  // is exclusive from (3).

  vul_arg< std::string > xgtf_timestamps_fn;
  vul_arg< std::string > xgtf_base_ts;

  vul_arg< bool > paired_gtct;
  vul_arg< bool > promote_pvmoving;
  vul_arg< std::string > qid;

  vul_arg< std::string > track_style_filter;
  vul_arg< bool > ts_from_fn;
  vul_arg< std::string > apix_debug_fn;

  // this argument is a string "M:N", M and N both integers >= 0 (e.g. "4:9".)
  // M and N are used to filter truth and computed tracks, respectively; only tracks
  // with at least that many states are kept.  Defaults to 0:0 (all tracks.)

  vul_arg< std::string > track_length_filter;

  // Define a time window in either frames or timestamp_usecs; only keep tracks
  // wholly within this window.
  vul_arg< std::string > time_window;

  // Some file formats, e.g. CSV, do not have a fixed source for the latitude
  // and longitude information required for setting MGRS data for radial overlap
  // computation.  Allow the user to specify these fields as a colon-separated
  // pair of strings, e.g. "longitude:latitude" or "world_y:world_x"
  vul_arg< std::string > mgrs_lon_lat_fields;

  // If set, assume kw18 will have a 19th column read into the relevancy slot
  // for each frame. (Computed tracks only.)
  vul_arg< bool > kw19_hack;

  // When scoring detections, we don't have a separate detection-only
  // data structure; just break them up into single-frame tracks.
  vul_arg< bool > detection_mode;

  // this flag is not set directly by an input_args command line variable,
  // but instead is set by the main program via other variables (such as
  // e.g. --radial-overlap).  When set, process() tries to compute MGRS geolocation
  // data (currently available only from APIX or KW18-with-world-coords.) If
  // it fails to do so, process() returns false.

  bool compute_mgrs_data;


  input_args_type()
    : computed_tracks_fn( "--computed-tracks", "Computed tracks file, or @filelist reads list of files" ),
      truth_tracks_fn(    "--truth-tracks", "Truth tracks files, scenario, or @filelist reads list of files" ),
      computed_format(    "--computed-format", "Disable computed format auto-detection; force to be this format; 'help' to list" ),
      truth_format(       "--truth-format", "Disable truth format auto-detection; force to be this format; 'help' to list" ),
      computed_path(      "--computed-path", "All computed files will be loaded from this directory, overriding other paths", "" ),
      truth_path(         "--truth-path", "All truth files will be loaded from this directory, overriding other paths", "" ),
      computed_fps(       "--computed-fps", "Computed tracks frames-per-second", 29.97 / 3 ),
      truth_fps(          "--truth-fps", "Truth tracks frames-per-second", 29.97 ),
      xgtf_timestamps_fn( "--xgtf-ts-file", "File of XGTF timestamps (format: xgtf-basename, fps, timestamp)" ),
      xgtf_base_ts(       "--xgtf-base-ts", "Base xgtf timestamp (usecs), unset to probe computed tracks, or 'probe' to probe, report, and exit" ),
      paired_gtct(        "--paired-gtct", "Require paired gt/ct and fabricate timestamps to separate the sets (excludes xgtf-ts options)" ),
      promote_pvmoving(   "--promote-pvmoving", "Set if XGTF activities should be promoted to PVMoving" ),
      qid(                "--qid", "The query ID to read computed tracks from", "" ),
      track_style_filter( "--track-style", "For KWXML, keep only tracks of the named style (both ground truth and computed" ),
      ts_from_fn(         "--fn2ts", "For any file format which does not define timestamps, convert frame number to timestamps (in seconds)" ),
      apix_debug_fn(      "--apix-log", "For APIX tracks, log tracks as read to this file", "" ),
      track_length_filter("--track-length-filter", "Only keep (truth:computed) tracks with at least this many states (default: all tracks)", "0:0" ),
      time_window(        "--time-window", "Only select tracks within a time window; 'help' for more details" ),
      mgrs_lon_lat_fields("--mgrs-ll-fields", "For e.g. CSV files, pull longitude / latitude from these fields", "world_x:world_y" ),
      kw19_hack(          "--kw19-hack", "If set, read confidence / probability / etc. from 19th column (computed only)" ),
      detection_mode(     "--detection-mode", "Convert truth and computed tracks to single-frame tracks to score as detections" ),
      compute_mgrs_data( false )
  {}


  bool sanity_check();

  bool process( kwiver::track_oracle::track_handle_list_type& computed_tracks,
                kwiver::track_oracle::track_handle_list_type& truth_tracks );

};

} // ...kwant
} // ...kwiver

#endif

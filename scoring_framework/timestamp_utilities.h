/*ckwg +5
 * Copyright 2013-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// Small collection of static utilities for use with timestamps.

#ifndef INCL_TIMESTAMP_UTILITIES_H
#define INCL_TIMESTAMP_UTILITIES_H

#include <vital/vital_config.h>
#include <scoring_framework/timestamp_utilities_export.h>

#include <ostream>
#include <utility>
#include <track_oracle/core/track_oracle_api_types.h>
#include <scoring_framework/score_core.h>

namespace kwiver {
namespace kwant {

namespace timestamp_utilities {

namespace kwto = ::kwiver::track_oracle;

//
// This object records the minimum and maxmimum values of timestamps
// and frame numbers for arbitrary collections of tracks.
//
// This seems to be an intersection of two recurring patterns: a
// frame iterator / visitor (aka the three set_from_* methods) and
// an algorithm (min/max) applied for (in this case) two track_fields.
//
// The implementation needs to be stricter: it allows a set of frames
// which have intermittent frame_numbers and/or timestamps, which could
// violate the timestamp_generator_factory's assumption that the min_ts
// and min_frame_number refer to the same frame.  Hmm.
//

struct TIMESTAMP_UTILITIES_EXPORT track_timestamp_stats_type
{
private:
  kwto::track_field< ts_type > timestamp_usecs;
  kwto::track_field< unsigned > frame_number;

public:
  track_timestamp_stats_type();
  track_timestamp_stats_type( const track_timestamp_stats_type& other );
  explicit track_timestamp_stats_type( const kwto::track_handle_list_type& tracks );
  explicit track_timestamp_stats_type( const kwto::track_handle_type& t );
  explicit track_timestamp_stats_type( const kwto::frame_handle_list_type& f );
  void combine_with_other( const track_timestamp_stats_type& other );
  void set_from_tracks( const kwto::track_handle_list_type& tracks );
  void set_from_track( const kwto::track_handle_type& t );
  void set_from_frames( const kwto::frame_handle_list_type& frames );
  void update_from_frame( const kwto::frame_handle_type& f );
  void reset();

  bool is_empty;
  bool has_frame_numbers;
  bool has_timestamps;
  bool all_have_frame_numbers;
  bool all_have_timestamps;
  std::pair<unsigned, unsigned> minmax_fn;
  std::pair<ts_type, ts_type> minmax_ts;
  std::pair<size_t, size_t> ts_fn_count;

};

TIMESTAMP_UTILITIES_EXPORT std::ostream& operator<<( std::ostream& os, const track_timestamp_stats_type& tts );

//
// A timestamp_generator writes (or re-writes) the timestamps for a
// track (or tracks) based on the assertion that frame 0 should have
// the value of [base_ts] and then extrapolating linearly forward
// based on the [fps] rate.
//
// Various ways to initialize a timestamp_generator are found in
// the timestamp_generator_factory.
//
// We use this to assign fabricated timestamps to track formats which do not
// have them, for example, XGTF.  Currently, we need to do this because we
// align tracks based on timestamp rather than frame number.  (When the global
// alignment branch lands, this will become less critical when we can align
// on arbitrary fields.)
//
// Note the reappearance of the frame visitor pattern in the three set_*
// methods.
//

class TIMESTAMP_UTILITIES_EXPORT timestamp_generator
{
public:

  timestamp_generator();
  timestamp_generator( double fps, ts_type base_ts );
  void set( double fps, ts_type base_ts );
  ts_type fn_to_ts( unsigned fn ) const;
  ts_type get_base_ts() const;
  void set_timestamps( const kwto::track_handle_list_type& tracks ) const;
  void set_timestamps( const kwto::track_handle_type& track ) const;
  void set_timestamps( const kwto::frame_handle_list_type& frames ) const;

private:
  mutable kwto::track_field< ts_type > timestamp_usecs;
  mutable kwto::track_field< unsigned > frame_number;
  bool valid;
  double fps;
  ts_type base_ts; // usecs
};

// When loading from a timebase file, we only remember the basename
// (because the the only way to use a timebase file is with an @file of
// full paths to the files.)  When we load a VIRAT scenario, we pass out
// the complete pathname, because the scenario provides both the timestamps
// and the full paths; the input_source_type copies this full path to its
// src_fn member and then resets this to basename(full_path) for uniformity
// of lookups.  (Whew.)

typedef std::map< std::string, timestamp_generator > timestamp_generator_map_type;
typedef std::map< std::string, timestamp_generator >::const_iterator timestamp_generator_cit;

struct TIMESTAMP_UTILITIES_EXPORT timestamp_generator_factory
{
  // create a timestamp_generator from a track_timestamp_stats (TTS)
  // object requires two FPS values: one for the generator itself, one
  // to use with the TTS to backtrack to the base timestamp
  static timestamp_generator from_tts( const track_timestamp_stats_type& tts,
                                       double fps_tts,
                                       double fps );

  // create a timestamp_generator_map from a timebase.dat file.
  // Returns a pair of (bool, map of filename -> timestamp_generator);
  // the bool is true if the map is valid, false if not (if, for example,
  // the file couldn't be read or something like that.)
  static std::pair< bool, timestamp_generator_map_type > from_timebase_file( const std::string& fn );

  // Given the URI and time strings from a VIRAT scenario GroundTruth element,
  // insert the corresponding timestamp_generator into the map.
  static bool from_virat_scenario( const std::string& uri,
                                   const std::string& start_ts_str,
                                   double fps,
                                   timestamp_generator_map_type& m );

};

} // ...timestamp_utilities

} // ...kwant
} // ...kwiver

#endif

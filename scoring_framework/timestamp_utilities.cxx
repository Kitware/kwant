/*ckwg +5
 * Copyright 2013-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "timestamp_utilities.h"

#include <fstream>
#include <algorithm>
#include <stdexcept>

#include <vul/vul_file.h>
#include <vul/vul_sprintf.h>
#include <vul/vul_reg_exp.h>

#include <track_oracle/core/track_field.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::getline;
using std::ifstream;
using std::istringstream;
using std::make_pair;
using std::max;
using std::min;
using std::ostream;
using std::pair;
using std::runtime_error;
using std::string;

using kwiver::track_oracle::track_handle_type;
using kwiver::track_oracle::track_handle_list_type;
using kwiver::track_oracle::frame_handle_type;
using kwiver::track_oracle::frame_handle_list_type;
using kwiver::track_oracle::track_oracle_core;
using kwiver::track_oracle::oracle_entry_handle_type;

namespace kwiver {

namespace kwant {

namespace timestamp_utilities {

track_timestamp_stats_type
::track_timestamp_stats_type()
  : timestamp_usecs( "timestamp_usecs" ),
    frame_number( "frame_number" )
{
  this->reset();
}

track_timestamp_stats_type
::track_timestamp_stats_type( const track_timestamp_stats_type& other )
  : timestamp_usecs( "timestamp_usecs" ),
    frame_number( "frame_number" ),
    is_empty( other.is_empty ),
    has_frame_numbers( other.has_frame_numbers ),
    has_timestamps( other.has_timestamps ),
    all_have_frame_numbers( other.all_have_frame_numbers ),
    all_have_timestamps( other.all_have_timestamps ),
    minmax_fn( other.minmax_fn ),
    minmax_ts( other.minmax_ts ),
    ts_fn_count( other.ts_fn_count )
{
}


void
track_timestamp_stats_type
::reset()
{
  this->is_empty = true;
  this->has_timestamps = false;
  this->has_frame_numbers = false;
  this->all_have_timestamps = true;
  this->all_have_frame_numbers = true;
  this->ts_fn_count = make_pair( 0, 0 );
}

track_timestamp_stats_type
::track_timestamp_stats_type( const track_handle_list_type& tracks )
  : timestamp_usecs( "timestamp_usecs" ),
    frame_number( "frame_number" )
{
  this->reset();
  this->set_from_tracks( tracks );
}

track_timestamp_stats_type
::track_timestamp_stats_type( const track_handle_type& t )
  : timestamp_usecs( "timestamp_usecs" ),
    frame_number( "frame_number" )
{
  this->reset();
  this->set_from_track( t );
}

track_timestamp_stats_type
::track_timestamp_stats_type( const frame_handle_list_type& f )
  : timestamp_usecs( "timestamp_usecs" ),
    frame_number( "frame_number" )
{
  this->reset();
  this->set_from_frames( f );
}

void
track_timestamp_stats_type
::combine_with_other( const track_timestamp_stats_type& other )
{
  // quick exit if other is empty
  if (other.is_empty)
  {
    return;
  }

  // quick copy if we're empty
  if (this->is_empty)
  {
    this->is_empty = other.is_empty;
    this->has_timestamps = other.has_timestamps;
    this->has_frame_numbers = other.has_frame_numbers;
    this->all_have_timestamps = other.all_have_timestamps;
    this->all_have_frame_numbers = other.all_have_frame_numbers;
    this->minmax_fn = other.minmax_fn;
    this->minmax_ts = other.minmax_ts;
    this->ts_fn_count = other.ts_fn_count;

    return;
  }

  //
  // both this and other are not empty
  //

  if (other.has_timestamps)
  {
    if ( ! this->has_timestamps )
    {
      this->minmax_ts = other.minmax_ts;
      this->has_timestamps = true;
    }
    else
    {
      this->minmax_ts.first = min( this->minmax_ts.first, other.minmax_ts.first );
      this->minmax_ts.second = max( this->minmax_ts.second, other.minmax_ts.second );
    }
  }

  if (other.has_frame_numbers)
  {
    if ( ! this->has_frame_numbers )
    {
      this->minmax_fn = other.minmax_fn;
      this->has_frame_numbers = true;
    }
    else
    {
      this->minmax_fn.first = min( this->minmax_fn.first, other.minmax_fn.first );
      this->minmax_fn.second = max( this->minmax_fn.second, other.minmax_fn.second );
    }
  }

  this->all_have_timestamps = this->all_have_timestamps && other.all_have_timestamps;
  this->all_have_frame_numbers = this->all_have_frame_numbers && other.all_have_frame_numbers;
}

void
track_timestamp_stats_type
::set_from_tracks( const track_handle_list_type& tracks )
{
  for (size_t i=0; i<tracks.size(); ++i)
  {
    this->set_from_track( tracks[i] );
  }
}

void
track_timestamp_stats_type
::set_from_track( const track_handle_type& t )
{
  this->set_from_frames( track_oracle_core::get_frames( t ));
}

void
track_timestamp_stats_type
::set_from_frames( const frame_handle_list_type& frames )
{
  for (size_t i=0; i<frames.size(); ++i)
  {
    this->update_from_frame( frames[i] );
  }
}

void
track_timestamp_stats_type
::update_from_frame( const frame_handle_type& f )
{
  const oracle_entry_handle_type& row = f.row;
  this->is_empty = false;

  pair< bool, ts_type > ts_probe = this->timestamp_usecs.get( row );
  if ( ts_probe.first )
  {
    ++this->ts_fn_count.first;
    ts_type v = ts_probe.second;
    if ( ! this->has_timestamps )
    {
      this->has_timestamps = true;
      this->minmax_ts = make_pair( v, v );
    }
    else
    {
      this->minmax_ts.first = min( this->minmax_ts.first, v );
      this->minmax_ts.second = max( this->minmax_ts.second, v );
    }
  }
  else
  {
    this->all_have_timestamps = false;
  }

  pair< bool, unsigned > fn_probe = this->frame_number.get( row );
  if ( fn_probe.first )
  {
    ++this->ts_fn_count.second;
    unsigned v = fn_probe.second;
    if ( ! this->has_frame_numbers )
    {
      this->has_frame_numbers = true;
      this->minmax_fn = make_pair( v, v );
    }
    else
    {
      this->minmax_fn.first = min( this->minmax_fn.first, v );
      this->minmax_fn.second = max( this->minmax_fn.second, v );
    }
  }
  else
  {
    this->all_have_frame_numbers = false;
  }
}

ostream&
operator<<( ostream& os,
            const track_timestamp_stats_type& tts )
{
  if (tts.is_empty)
  {
    os << "Is_empty";
  }
  else
  {
    os << "Has fn/ts? " << tts.has_frame_numbers << " " << tts.has_timestamps
       << " ; all have fn/ts? " << tts.all_have_frame_numbers << " " << tts.all_have_timestamps
       << " ;";
    os << " #ts/fn: " << tts.ts_fn_count.first << " : "
       << tts.ts_fn_count.second << " ; ";
    os << " fn range ";
    if (tts.has_frame_numbers)
    {
      os << tts.minmax_fn.first << " : " << tts.minmax_fn.second << " ";
    }
    else
    {
      os << " na ";
    }
    if (tts.has_timestamps)
    {
      os << tts.minmax_ts.first << " : " << tts.minmax_ts.second;
    }
    else
    {
      os << " na";
    }
  }
  return os;
}


timestamp_generator
::timestamp_generator()
  : timestamp_usecs( "timestamp_usecs" ),
    frame_number( "frame_number" ),
    valid( false ),
    fps( 0.0 ),
    base_ts( 0 )
{
}

timestamp_generator
::timestamp_generator( double f, ts_type b )
  : timestamp_usecs( "timestamp_usecs" ),
    frame_number( "frame_number" ),
    valid( true ),
    fps( f ),
    base_ts( b )
{
}

ts_type
timestamp_generator
::fn_to_ts( unsigned fn ) const
{
  if ( ! this->valid ) throw runtime_error( "timestamp_generator::fn_to_ts with invalid generator");
  double seconds_of_frames = fn / this->fps;
  ts_type usecs_of_frames = static_cast< ts_type >( seconds_of_frames * 1.0e6 );
  return this->base_ts + usecs_of_frames;
}

ts_type
timestamp_generator
::get_base_ts() const
{
  if ( ! this->valid ) throw runtime_error( "timestamp_generator::base_to_ts with invalid generator");
  return this->base_ts;
}

void
timestamp_generator
::set_timestamps( const track_handle_list_type& tracks ) const
{
  for (size_t i=0; i<tracks.size(); ++i)
  {
    this->set_timestamps( tracks[i] );
  }
}

void
timestamp_generator
::set_timestamps( const track_handle_type& track ) const
{
  this->set_timestamps( track_oracle_core::get_frames( track ));
}

void
timestamp_generator
::set_timestamps( const frame_handle_list_type& frames ) const
{
  for (size_t i=0; i<frames.size(); ++i)
  {
    pair< bool, unsigned > probe = this->frame_number.get( frames[i].row );
    if ( probe.first )
    {
      this->timestamp_usecs( frames[i].row ) = this->fn_to_ts( probe.second );
    }
    else
    {
      throw runtime_error( "timestamp_generator: tried to set a timestamp on a frame without a frame_number" );
    }
  }
}

timestamp_generator
timestamp_generator_factory
::from_tts( const track_timestamp_stats_type& tts,
            double fps_tts,
            double fps )
{
  // can only calculate from the tts if it has both frame numbers and
  // timestamps
  if ( tts.is_empty || (! ( tts.has_frame_numbers && tts.has_timestamps )))
  {
    LOG_ERROR( main_logger, "timestamp_generator from_tts: tts needs both frame numbers and timestamps" );
    return timestamp_generator();
  }

  // take the minimum ts, assume it's referring to the same frame as the min frame number
  double offset_in_seconds_from_frame_zero = tts.minmax_fn.first / fps_tts;
  ts_type offset_in_usecs = static_cast< ts_type >( offset_in_seconds_from_frame_zero * 1.0e6 );

  if ( offset_in_usecs > tts.minmax_ts.first )
  {
    LOG_ERROR( main_logger, "timestamp_generator_factory::from_tts: min frame " << tts.minmax_fn.first
               << " @" << fps_tts << " fps has ts offset " << offset_in_usecs
               << " greater than min timestamp " << tts.minmax_ts.first
               << "; would result in a base_ts < 0; returning invalid timestamp_generator" );
    return timestamp_generator();
  }

  ts_type base_ts = tts.minmax_ts.first - offset_in_usecs;
  return timestamp_generator( fps, base_ts );
}


pair< bool, timestamp_generator_map_type >
timestamp_generator_factory
::from_timebase_file( const string& fn )
{
  ifstream is( fn.c_str() );
  if ( ! is )
  {
    LOG_ERROR( main_logger, "Couldn't open timebase file '" << fn << "'" );
    return make_pair( false, timestamp_generator_map_type() );
  }
  timestamp_generator_map_type m;
  string tmp;
  while (getline(is, tmp))
  {
    istringstream iss(tmp);
    string tsfn;
    double fps;
    ts_type base_ts;
    if ( ! ( iss >> tsfn >> fps >> base_ts ))
    {
      LOG_ERROR( main_logger, "Couldn't parse fn / fps / base_ts from '" << tmp << "'");
      return make_pair( false, timestamp_generator_map_type() );
    }

    tsfn = vul_file::basename( tsfn );
    m[ tsfn ] = timestamp_generator( fps, base_ts );
  }
  return make_pair( true, m );
}


bool
timestamp_generator_factory
::from_virat_scenario( const string& uri,
                       const string& start_ts_str,
                       double fps,
                       timestamp_generator_map_type& m )
{
  // timestamp expected format expected as e.g. "2009-03-19T18:26:59.666Z"
  vul_reg_exp re_ts("([0-9]+)\\-([0-9]+)\\-([0-9]+)T([0-9]+):([0-9]+):([0-9]+)\\.([0-9]+)Z");
  if ( ! re_ts.find( start_ts_str ))
  {
    LOG_ERROR( main_logger, "Couldn't parse a scenario timestamp for " << uri << " from '" << start_ts_str << "'");
    return false;
  }

  string time_str(
    vul_sprintf( "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                 boost::lexical_cast<int>( re_ts.match(1) ),    // year
                 boost::lexical_cast<int>( re_ts.match(2) ),    // month
                 boost::lexical_cast<int>( re_ts.match(3) ),    // day
                 boost::lexical_cast<int>( re_ts.match(4) ),    // hour
                 boost::lexical_cast<int>( re_ts.match(5) ),    // minute
                 boost::lexical_cast<int>( re_ts.match(6) ),    // second
                 boost::lexical_cast<int>( re_ts.match(7) )).c_str() );  // fractional seconds

  boost::posix_time::ptime ts( boost::posix_time::time_from_string(time_str) );
  boost::posix_time::ptime epoch( boost::gregorian::date(1970,1,1));
  boost::posix_time::time_duration diff = ts - epoch;

  m[ uri ] = timestamp_generator( fps, diff.total_microseconds() );

  return true;
}

} // ...timestamp_utilities

} // ...kwant
} // ...kwiver

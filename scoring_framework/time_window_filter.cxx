/*ckwg +5
 * Copyright 2013-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "time_window_filter.h"

#include <stdexcept>
#include <algorithm>
#include <limits>

#include <vul/vul_reg_exp.h>

#include <track_oracle/track_oracle.h>
#include <track_oracle/track_field.h>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::istringstream;
using std::numeric_limits;
using std::ostringstream;
using std::runtime_error;
using std::string;
using std::swap;

namespace { // anon

unsigned long long
parse_window_bound( const string& s,
                    const string& unit_code,
                    unsigned long long default_value )
{
  if (s.empty()) return default_value;

  unsigned long long ret;
  istringstream iss( s );
  if ( (unit_code == "f") ||  // frames
       (unit_code == "t") )   // usecs
  {
    if ( ! ( iss >> ret ))
    {
      throw runtime_error( "Couldn't parse time window bound from '"+s+"' (unit code " + unit_code +") ?" );
    }
  }
  else if (unit_code == "T") // secs
  {
    double tmp;
    if ( ! ( iss >> tmp ))
    {
      throw runtime_error( "Couldn't parse time window bound from '"+s+"' (unit code " + unit_code +") ?" );
    }
    ret = static_cast< unsigned long long >( tmp * 1.0e6 );
  }
  else
  {
    throw runtime_error( "Bad time window unit code '"+s+"'; expected one of [tfF]" );
  }

  return ret;
}

} // anon

namespace kwiver {
namespace kwant {

string
time_window_filter
::help_text()
{
  return string(
    "A time window specifies that only tracks within the given time\n"
    "bound are scored.  This time window applies to both computed and\n"
    "ground-truth tracks.\n"
    "\n"
    "Windows may be either 'inclusive' or 'exclusive'.  An inclusive window\n"
    "will accept any track with at least one state within the window.  An exclusive\n"
    "window will only accept tracks whose every state is within the window.\n"
    "\n"
    "Time window filtering occurs BEFORE any track-minimum-length filters are\n"
    "applied.\n"
    "\n"
    "The format is: '(flag)(field)[min]:[max]' ; flag, field and the colon are mandatory;\n"
    "min and max are optional; if left out, lowest and highest values in the\n"
    "dataset are used.\n"
    "\n"
    "Flag values are:\n"
    "'i' for inclusive\n"
    "'x' for exclusive\n"
    "\n"
    "Field values are:\n"
    "'f' for frame numbers\n"
    "'t' / 'T' for timestamps in usecs / secs\n"
    "\n"
    "Examples:\n"
    "'if100:300' keeps all tracks with states within frames 100-300.\n"
    "'xt:6150002' keeps all tracks wholly contained from the beginning to 6150002 *usecs*.\n"
    "'iT500:' keeps all tracks at least partially contained from 500 *seconds* to the end.\n"
    "'it:' keeps all tracks, and is identical to 'iT:' and 'if:' (and exclusive versions.)\n" );
}

time_window_filter
::time_window_filter()
  : valid(false)
{
}

bool
time_window_filter
::set_from_string( const string& s )
{
  vul_reg_exp re( "^([ix])([ftT])([0-9\\.]*):([0-9\\.]*)$" );

  this->valid = false;
  if (re.find( s ))
  {
    this->inclusive = (re.match(1) == "i" );
    this->units_are_frames = (re.match(2) == "f");
    this->min = parse_window_bound( re.match(3), re.match(2), numeric_limits< unsigned long long >::min() );
    this->max = parse_window_bound( re.match(4), re.match(2), numeric_limits< unsigned long long >::max() );
    if (this->min > this->max)
    {
      LOG_INFO( main_logger, "time_window_filter min / max " << this->min << " / " << this->max <<
                " out of order; swapping" );
      swap( this->min, this->max );
    }
    this->valid = true;
  }
  else
  {
    LOG_ERROR( main_logger, "Couldn't set time window filter from '" << s << "'" );
  }
  return this->valid;
}

bool
time_window_filter
::track_passes_filter( const track_handle_type& t ) const
{
  track_field< unsigned long long > ts_usecs( "timestamp_usecs" );
  track_field< unsigned > ts_frame( "frame_number" );

  frame_handle_list_type frames = track_oracle::get_frames( t );
  for (size_t i=0; i<frames.size(); ++i)
  {
    const oracle_entry_handle_type& row = frames[i].row;

    bool in_window;

    if (this->units_are_frames)
    {
      if ( ! ts_frame.exists( row ))
      {
        LOG_WARN( main_logger, "Time window is on frames but track does not have frame numbers?  Rejecting" );
        return false;
      }
      unsigned frame_number = ts_frame( row );
      in_window = (this->min <= frame_number) && (frame_number <= this->max);
    }
    else
    {
      if ( ! ts_usecs.exists( row ))
      {
        LOG_WARN( main_logger, "Time window is on timestamps but track does not have timestamps?  Rejecting" );
        return false;
      }
      unsigned long long ts = ts_usecs( row );
      in_window = (this->min <= ts) && (ts <= this->max);
    }
    if ( ( ! in_window ) && ( ! this->inclusive ))
    {
      // we're outside the window, and the window is exclusive; return false
      return false;
    }
    if ( in_window && this->inclusive )
    {
      // we're inside the window, and the window is inclusive-- return true
      return true;
    }
  } // each frame

  if (this->inclusive)
  {
    // if the window is inclusive, and we get here, then every frame was outside the window-- return false
    return false;
  }
  else
  {
    // if the window is exclusive, and we got here, then every frame is inside the window-- return true;
    return true;
  }
}

bool
time_window_filter
::is_valid() const
{
  return this->valid;
}

string
time_window_filter_factory
::help_text()
{
  return "The special values 'G', 'g', 'C', 'c', 'M', and 'm' are also accepted; these autocompute to\n"
    "the timestamp bounds of (G)roundtruth or (C)omputed.  'G' and 'C' produce\n"
    "exclusive windows (every state must be in the window); 'g' and 'c' produce\n"
    "inclusive window (any state may be in the window).\n"
    "'M' and 'm' produce exclusive and inclusive windows, respectively, based on the\n"
    "maximum lowest and minimum highest timestamp between the computed and ground-truth sets.\n";
}

bool
time_window_filter_factory
::code_is_special( const string& code )
{
  return
      (code == "G") ||
      (code == "C") ||
      (code == "g") ||
      (code == "c") ||
      (code == "M") ||
      (code == "m");
}

time_window_filter
time_window_filter_factory
::from_stats( const string& code,
              const timestamp_utilities::track_timestamp_stats_type& tstats,
              const timestamp_utilities::track_timestamp_stats_type& cstats )
{
  ostringstream oss;
  char flag;
  if ((code == "G") || (code == "g"))
  {
    if (code == "G")
    {
      LOG_INFO( main_logger, "Time window filter will be EXCLUSIVE and derived from ground-truth..." );
      flag = 'x';
    }
    else
    {
      LOG_INFO( main_logger, "Time window filter will be INCLUSIVE and derived from ground-truth..." );
      flag = 'i';
    }
    oss << flag << "t" << tstats.minmax_ts.first << ":" << tstats.minmax_ts.second;
    LOG_INFO( main_logger, "Advisory time window frames " << tstats.minmax_fn.first << " : " << tstats.minmax_fn.second );
  }
  else if ((code == "C") || (code == "c" ))
  {
    if (code == "C")
    {
      LOG_INFO( main_logger, "Time window filter will be EXCLUSIVE and derived from computed..." );
      flag = 'x';
    }
    else
    {
      LOG_INFO( main_logger, "Time window filter will be INCLUSIVE and derived from computed..." );
      flag = 'i';
    }
    oss << flag << "t" << cstats.minmax_ts.first << ":" << cstats.minmax_ts.second;
    LOG_INFO( main_logger, "Advisory time window frames " << cstats.minmax_fn.first << " : " << cstats.minmax_fn.second );
  }
  else if ((code == "M") || (code == "m" ))
  {
    if (code == "M")
    {
      LOG_INFO( main_logger, "Time window filter will be EXCLUSIVE and derived from the minimum window of both track sets..." );
      flag = 'x';
    }
    else
    {
      LOG_INFO( main_logger, "Time window filter will be INCLUSIVE and derived from the minimum window of both track sets..." );
      flag = 'i';
    }
    ts_type lower_bound, upper_bound;
    unsigned lower_bound_frame, upper_bound_frame;
    if (tstats.minmax_ts.first < cstats.minmax_ts.first )
    {
      lower_bound = cstats.minmax_ts.first;
      lower_bound_frame = cstats.minmax_fn.first;
      LOG_INFO( main_logger, "Using lower bound from computed; " << tstats.minmax_ts.first - lower_bound << " usecs ("
                << tstats.minmax_fn.first - lower_bound_frame << " frames) ahead of ground-truth ");
    }
    else
    {
      lower_bound = tstats.minmax_ts.first;
      lower_bound_frame = tstats.minmax_fn.first;
      LOG_INFO( main_logger, "Using lower bound from ground-truth; "  << cstats.minmax_ts.first - lower_bound << " usecs ("
                << cstats.minmax_fn.first - lower_bound_frame << " frames) ahead of computed ");
    }
    if (tstats.minmax_ts.second < cstats.minmax_ts.second )
    {
      upper_bound = tstats.minmax_ts.second;
      upper_bound_frame = tstats.minmax_fn.second;
      LOG_INFO( main_logger, "Using upper bound from ground-truth; " << cstats.minmax_ts.second - upper_bound << " usecs ("
                << cstats.minmax_fn.second - upper_bound_frame << " frames) below computed ");
    }
    else
    {
      upper_bound = cstats.minmax_ts.second;
      upper_bound_frame = cstats.minmax_fn.second;
      LOG_INFO( main_logger, "Using upper bound from computed..." << tstats.minmax_ts.second - upper_bound << " usecs ("
                << tstats.minmax_ts.second - upper_bound_frame << " frames) below ground-truth " );
    }
    if (upper_bound < lower_bound )
    {
      LOG_ERROR( main_logger, "Upper bound less than upper bound... are computed and ground-truth from the same run?" );
      return time_window_filter();
    }
    oss << flag << "t" << lower_bound << ":" << upper_bound;
    LOG_INFO( main_logger, "Advisory time window frames " << lower_bound_frame << " : " << upper_bound_frame );
  }
  else
  {
    LOG_ERROR( main_logger, "time_window_filter_factory called with invalid code '" << code << "'" );
    return time_window_filter();
  }
  time_window_filter twf;
  bool rc = twf.set_from_string( oss.str() );
  if ( ! rc ) throw runtime_error( "Couldn't set twf from code '" + code + "': '" + oss.str() + "'" );
  LOG_INFO( main_logger, "Set to '" << oss.str() << "'" );
  return twf;
}

} // ...kwant
} // ...kwiver

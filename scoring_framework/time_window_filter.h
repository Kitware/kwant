/*ckwg +5
 * Copyright 2013-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TIME_WINDOW_FILTER_H
#define INCL_TIME_WINDOW_FILTER_H

#include <vital/vital_config.h>
#include <scoring_framework/score_core_export.h>

#include <string>
#include <track_oracle/core/track_oracle_api_types.h>
#include <scoring_framework/timestamp_utilities.h>

namespace kwiver {
namespace kwant {

class SCORE_CORE_EXPORT time_window_filter
{
public:

  time_window_filter();
  bool set_from_string( const std::string& s );
  bool track_passes_filter( const kwiver::track_oracle::track_handle_type& t ) const;
  bool is_valid() const;
  static std::string help_text();

private:
  bool units_are_frames; // true if frames, false if timestamp_usecs
  bool inclusive; // true if inclusive, false if exclusive
  bool valid; // true if constructed

  // relying on the fact that unsigned long long can hold
  // both frames and timestamp_usecs
  unsigned long long min;
  unsigned long long max;
};

struct SCORE_CORE_EXPORT time_window_filter_factory
{
  static std::string help_text();
  static bool code_is_special( const std::string& code );
  static time_window_filter from_stats( const std::string& code,
                                        const timestamp_utilities::track_timestamp_stats_type& tstats,
                                        const timestamp_utilities::track_timestamp_stats_type& cstats );
};


} // ...kwant
} // ...kwiver

#endif

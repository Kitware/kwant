/*ckwg +5
 * Copyright 2013-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// Small collection of static utilities for use with VIRAT scenario files.

#ifndef INCL_VIRAT_SCENARIO_UTILITIES_H
#define INCL_VIRAT_SCENARIO_UTILITIES_H

#include <vital/vital_config.h>
#include <scoring_framework/score_core_export.h>

#include <scoring_framework/timestamp_utilities.h>

namespace kwiver {
namespace kwant {

namespace virat_scenario_utilities
{

// See if the file contains a virat scenario with a quick
// parse of the tokens, rather than a full-on tinyXML load

SCORE_CORE_EXPORT bool fn_is_virat_scenario( const std::string& fn );


// Assuming that fn is a virat scenario, parse it and return two
// pieces of information:
// - the query ID to activity map (for scoring)
// - the groundtruth filename / base timestamp map (for loading and aligning
//   groundtruth tracks

SCORE_CORE_EXPORT bool process( const std::string& fn,
                                std::map< std::string, std::vector< size_t > >& qid2activity_map,
                                timestamp_utilities::timestamp_generator_map_type& xgtf_map,
                                double fps );

} // virat_scenario_utilities

} // ...kwant
} // ...kwiver

#endif

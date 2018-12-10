/*ckwg +5
 * Copyright 2010-2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "matching_args_type.h"

#include <sstream>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::istringstream;
using std::pair;
using std::string;

namespace kwiver {
namespace kwant {

bool
matching_args_type
::sanity_check() const
{
  bool ret = true;
  bool use_radial = (this->radial_overlap() >= 0.0);
  bool use_spatial =
    this->bbox_expansion.set()
    || (this->min_bound_area() > 0.0)
    || this->min_pcent_gt_ct.set()
    || this->iou.set();
  if ( use_radial && use_spatial )
  {
    LOG_ERROR( main_logger, "Both spatial (bounding box) and radial overlap options have been set.\n" <<
               "Spatial and radial overlap testing are mutually exclusive; please select\n" <<
               "only one method.\n" );
    ret = false;
  }

  // non-const because vul_arg_base.option() is not const
  matching_args_type* mutable_me = const_cast< matching_args_type * >( this );

  if (this->min_pcent_gt_ct.set() && this->iou.set())
  {
    LOG_ERROR( main_logger, string("\n")
               << "Can't specify both " << mutable_me->min_pcent_gt_ct.option() << " and "
               << mutable_me->iou.option() << "\n\n" );
    ret = false;
  }

  // pass_nonzero_overlaps and radial_overlap don't make sense when specified together
  if ( use_radial && (this->pass_nonzero_overlaps()))
  {
    LOG_ERROR( main_logger, string("\n")
               << "Can't specify both " << mutable_me->radial_overlap.option() << " and "
               << mutable_me->pass_nonzero_overlaps.option() << ".\n\n"
               << "Passing non-zero overlaps allows separation\n"
               << "of spatial overlaps into two categories: a strong test used to decide\n"
               << "if the tracks overlap at all, and a weak test to determine if individual\n"
               << "detections overlap.  Radial overlaps are binary; we don't have a distinction\n"
               << "between 'strong' and 'weak' readial overlaps yet.\n\n" );
    ret = false;
  }

  string pcent_opt = mutable_me->min_pcent_gt_ct.option();
  string min_bounding_area_opt = mutable_me->min_bound_area.option();
  string min_filter_frames_opt = mutable_me->min_frames_arg.option();
  if (this->min_pcent_gt_ct() == "help")
  {
    LOG_ERROR( main_logger, string( "\n" ) <<
               "The " << pcent_opt << " option sets the minimum area, as a percentage of the box, which\n"
               "must be overlapped in either the ground-truth box, computed box, or both, for the\n"
               "overlap to count as a hit.\n\n"
               "If set, this option overrides the " << min_bounding_area_opt << " option.\n\n"
               "This option is specified as two numbers separated by a colon, e.g. '20.5:5'.\n"
               "The first is the percentage of the ground-truth box; the second is the percentage\n"
               "of the computed box.  To set one value as don't care, set it to 0; for example,\n"
               "'54.9:0' means as long as 54.9 percent of the ground-truth box is overlapped, any amount\n"
               "of overlap in the computed box is allowed; similarly, '0:10' means as long as 10 percent\n"
               "of the computed box is overlapped, any overlap in the ground-truth box is accepted.\n\n"
               "If the separating character is 'd' instead of ':', overlap area computations are dumped to cerr.\n"
               "\n"
               "If " << pcent_opt << " is specified, " << min_filter_frames_opt << " must be explicitly set\n"
               "to state how many frames must pass the " << pcent_opt << " criteria before the track-to-track\n"
               "overlap is accepted.\n" );
    ret = false;
  }

  if (this->aoi_string() == "help")
  {
    LOG_INFO( main_logger, string( "\n" ) <<
              "Spatial AOIs may be specified in one of three formats:\n"
              "\n"
              "1) Pixel AOI:        'WxH+x+y', e.g. '240x191+100+100'\n"
              "2) lon/lat 2-corner: 'NW_lon,NW_lat:SE_lon,SE_lat', e.g. '-72,43:-71,42'\n"
              "3) lon/lat 4-corner: 4 pairs of (lon,lat), e.g. '-72,43:-72,42:-71,42:-71,43'\n"
              "\n"
              "Pixel AOIs always construct axis-aligned bounding boxes.\n"
              "\n"
              "A lon/lat 2-corner box is expanded into a lon/lat-aligned 4-corner box\n"
              "and processed in the same way as arbitrary 4-corner boxes.\n"
              "\n"
              "A lon/lat 4-corner box is processed by converting each corner into UTM\n"
              "and constructing a convex hull, rather than an northing/easting aligned\n"
              "box.  If no single UTM zone contains all four corners, an exception is thrown.\n"
              "\n" );
    ret = false;
  }

  if (this->min_pcent_gt_ct.set() && (! this->min_frames_arg.set() ))
  {
    LOG_ERROR( main_logger, string("\n") <<
               "When using using the " << pcent_opt << " option, the " << min_filter_frames_opt << " option\n"
               "must be explicitly set.  Use 'help' as the argument to " << pcent_opt << " for details.\n" );
    ret = false;
  }

  return ret;
}

bool
matching_args_type
::parse_min_pcent_gt_ct( pair<double, double>& p, bool& debug_flag ) const
{
  istringstream iss( this->min_pcent_gt_ct() );
  double d1, d2;
  char c;
  if ( (iss >> d1 >> c >> d2))
  {
    p.first = d1;
    p.second = d2;
    debug_flag = (c == 'd');
    LOG_INFO( main_logger, "minimum gt/ct overlap computation enabled; gt percentage: " << p.first
              << "; ct percentage: " << p.second << "; debug mode: " << debug_flag );
    return true;
  }
  else
  {
    LOG_ERROR( main_logger, "Couldn't parse '" << this->min_pcent_gt_ct()
               << "' as a minimum gt/ct overlap argument; try 'help' for help" );
    return false;
  }
}

bool
matching_args_type
::parse_min_frames_arg()
{
  const string& s = this->min_frames_arg();
  size_t p = s.find_first_of( "p" );
  // the policy is absolute if no 'p' is in the string
  this->min_frames_policy.first = (p == string::npos);
  istringstream iss( s.substr( 0, p ));
  if ( ! ( iss >> this->min_frames_policy.second ))
  {
    LOG_ERROR( main_logger, "Couldn't parse min-frames value from '" << s.substr(0,p) << "'?" );
    return false;
  }
  return true;
}

} // ...kwant
} // ...kwiver

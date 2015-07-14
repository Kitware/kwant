/*ckwg +5
 * Copyright 2010-2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_GEOGRAPHIC_GEO_COORDS_H_
#define VIDTK_GEOGRAPHIC_GEO_COORDS_H_

/*
 * \file geo_coords.h
 * \brief This a a wrapper around the Geographic_lib::Geo_coords class.
 */

#include <string>

namespace vidtk
{
namespace geographic
{

// Sentinal value for lat / lon
static const double INVALID_LAT_LON = 444;

// Lightweight static function to check lat/lon coords
bool is_latlon_valid(const double latitude, const double longitude);

struct geo_coords_impl;

/**
   * \brief Conversion between geographic coordinates
   *
   * This class stores a geographic position which may be set via the
   * constructors or Reset via
   * - latitude and longitude
   * - UTM or UPS coordinates
   * - a string represention of these or an MGRS coordinate string
   *
   * The state consists of the latitude and longitude and the supplied UTM or
   * UPS coordinates (possibly derived from the MGRS coordinates).  If latitude
   * and longitude were given then the UTM/UPS coordinates follows the standard
   * conventions.
   *
   * The mutable state consists of the UTM or UPS coordinates for a alternate
   * zone.  A method Set_alt_zone is provided to set the alternate UPS/UTM zone.
   *
   * Methods are provided to return the geographic coordinates, the input UTM
   * or UPS coordinates (and associated meridian convergence and scale), or
   * alternate UTM or UPS coordinates (and their associated meridian
   * convergence and scale).
   *
   * Once the input string has been parsed, you can print the result out in any
   * of the formats, decimal degrees, degrees minutes seconds, MGRS, UTM/UPS.
   **********************************************************************/

class geo_coords
{
public:

  /**
   * The default contructor is equivalent to \e latitude = 90<sup>o</sup>, \e
   * longitude = 0<sup>o</sup>.
   **********************************************************************/
  geo_coords(void);

  /**
   * Parse as a string and interpret it as a geographic position.  The input
   * string is broken into space (or comma) separated pieces and Basic
   * decision on which format is based on number of components
   * -# MGRS
   * -# "Lat Long" or "Long Lat"
   * -# "Zone Easting Northing" or "Easting Northing Zone"
   *
   * The following inputs are approximately the same (Ar Ramadi Bridge, Iraq)
   * - Latitude and Longitude
   *   -  33.44      43.27
   *   -  N33d26.4'  E43d16.2'
   *   -  43d16'12"E 33d26'24"N
   * - MGRS
   *   -  38SLC301
   *   -  38SLC391014
   *   -  38SLC3918701405
   *   -  37SHT9708
   * - UTM
   *   -  38N 339188 3701405
   *   -  897039 3708229 37N
   *
   * Latitude and Longitude parsing.  Latitude precedes longitude, unless a
   * N, S, E, W hemisphere designator is used on one or both coordinates.
   * Thus
   * - 40 -75
   * - N40 W75
   * - -75 N40
   * - 75W 40N
   * - E-75 -40S
   * .
   * are all the same position.  The coodinates may be given in decimal
   * degrees, degrees and decimal minutes, degrees, minutes, seconds, etc.
   * Use d, ', and " to make off the degrees, minutes and seconds.  Thus
   * - 40d30'30"
   * - 40d30'30
   * - 40d30.5'
   * - 40d30.5
   * - 40.508333333
   * .
   * all specify the same angle.  The leading sign applies to all components
   * so -1d30 is -(1+30/60) = -1.5.  Latitudes must be in the range [-90, 90]
   * and longitudes in the range [-180, 360].  Internally longitudes are
   * reduced to the range [-180, 180).
   *
   * UTM/UPS parsing.  For UTM zones (-80 <= Lat <= 84), the zone designator
   * is made up of a zone number (for 1 to 60) and a hemisphere letter (N or
   * S), e.g., 38N.  The latitude zone designer ([C&ndash;M] in the southern
   * hemisphere and [N&ndash;X] in the northern) should NOT be used.  (This
   * is part of the MGRS coordinate.)  The zone designator for the poles
   * (where UPS is employed) is a hemisphere letter by itself, i.e., N or S.
   *
   * MGRS parsing interprets the grid references as square area at the
   * specified precision (1m, 10m, 100m, etc.).  If \e centerp = true (the
   * default), the center of this square is then taken to be the precise
   * position; thus:
   * - 38SMB           = 38N 450000 3650000
   * - 38SMB4484       = 38N 444500 3684500
   * - 38SMB44148470   = 38N 444145 3684705
   * .
   * Otherwise, the "south-west" corner of the square is used, i.e.,
   * - 38SMB           = 38N 400000 3600000
   * - 38SMB4484       = 38N 444000 3684000
   * - 38SMB44148470   = 38N 444140 3684700
   **********************************************************************/
  explicit geo_coords(const std::string &s);

  /**
   * Specify the location in terms of \e latitude (degrees) and \e longitude
   * (degrees).
   **********************************************************************/
  geo_coords(double latitude, double longitude);

  /**
   * Specify the location in terms of UTM/UPS \e zone (zero means UPS),
   * hemisphere \e is_north (false means south, true means north), \e easting
   * (meters) and \e northing (meters).
   **********************************************************************/
  geo_coords(int zone, bool is_north, double easting, double northing);

  geo_coords (geo_coords const & obj); // copy constructor
  ~geo_coords();

  geo_coords & operator = (geo_coords const & obj); // assignment operator

  /**
   * Reset the location as a 1-element, 2-element, or 3-element string.  See
   * geo_coords(const string& s).
   **********************************************************************/
  bool reset(const std::string& s);

  /**
   * Reset the location in terms of \e latitude and \e longitude.  See
   * igeo_oords(real latitude, real longitude, int zone).
   **********************************************************************/
  bool reset(double latitude, double longitude);

  /**
   * Reset the location in terms of UPS/UPS \e zone, hemisphere \e northp, \e
   * easting, and \e northing.  See geo_coords(int zone, bool northp,
   * real easting, real northing).
   **********************************************************************/
  bool reset(int zone, bool is_north, double easting, double northing);

  /**
   * Return latitude (degrees)
   **********************************************************************/
  double latitude(void) const;

  /**
   * Return longitude (degrees)
   **********************************************************************/
  double longitude(void) const;

  /**
   * Return easting (meters)
   **********************************************************************/
  double easting(void) const;

  /**
   * Return northing (meters)
   **********************************************************************/
  double northing(void) const;

  /**
   * Return meridian convergence (degrees) for the UTM/UPS projection.
   **********************************************************************/
  double convergence(void) const;

  /**
   * Return scale for the UTM/UPS projection.
   **********************************************************************/
  double scale(void) const;

  /**
   * Return hemisphere (false means south, true means north).
   **********************************************************************/
  bool is_north(void) const;

  /**
   * Return hemisphere letter N or S.
   **********************************************************************/
  char hemisphere(void) const;

  /**
   * Return the zone corresponding to the input (return 0 for UPS).
   **********************************************************************/
  int zone() const;

  /**
   * Use zone number, \e zone, for the alternate representation.  See
   * UTMUPS::zonespec for more information on the interpretation of \e zone.
   * Note that \e zone == UTMUPS::STANDARD (the default) use the standard UPS
   * or UTM zone, UTMUPS::MATCH does nothing retaining the existing alternate
   * representation.  Before this is called the alternate zone is the input
   * zone.
   **********************************************************************/
  bool set_alt_zone(int zone);

  /**
   * Returns the current alternate zone (return 0 for UPS).
   **********************************************************************/
  int alt_zone(void) const;

  /**
   * Return easting (meters) for alternate zone.
   **********************************************************************/
  double alt_easting() const;

  /**
   * Return northing (meters) for alternate zone.
   **********************************************************************/
  double alt_northing() const;

  /**
   * Return meridian convergence (degrees) for altermate zone.
   **********************************************************************/
  double alt_convergence() const;

  /**
   * Return scale for altermate zone.
   **********************************************************************/
  double alt_scale() const;

  /**
   * Return string with latitude and longitude as signed decimal degrees.
   * Precision \e prec specifies accuracy of representation as follows:
   * - prec = -5 (min), 1d
   * - prec = 0, 10<sup>-5</sup>d (about 1m)
   * - prec = 3, 10<sup>-8</sup>d
   * - prec = 9 (max), 10<sup>-14</sup>d
   **********************************************************************/
  std::string geo_representation(int prec = 0) const;

  /**
   * Return string with latitude and longitude as degrees, minutes, seconds,
   * and hemisphere.  Precision \e prec specifies accuracy of representation
   * as follows:
   * - prec = -5 (min), 1d
   * - prec = -4, 0.1d
   * - prec = -3, 1'
   * - prec = -2, 0.1'
   * - prec = -1, 1"
   * - prec = 0, 0.1" (about 3m)
   * - prec = 1, 0.01"
   * - prec = 10 (max), 10<sup>-11</sup>"
   **********************************************************************/
  std::string dms_representation(int prec = 0) const;

  /**
   * Return MGRS string.  This gives the coordinates of the enclosing grid
   * square with size given by the precision \e prec.  Thus 38N 444180
   * 3684790 converted to a MGRS coordinate at precision -2 (100m) is
   * 38SMB441847 and not 38SMB442848.  Precision \e prec specifies the
   * precision of the MSGRS string as follows:
   * - prec = -5 (min), 100km
   * - prec = -4, 10km
   * - prec = -3, 1km
   * - prec = -2, 100m
   * - prec = -1, 10m
   * - prec = 0, 1m
   * - prec = 1, 0.1m
   * - prec = 6 (max), 1um
   **********************************************************************/
  std::string mgrs_representation(int prec = 0) const;

  /**
   * Return string consisting of UTM/UPS zone designator, easting, and
   * northing,  Precision \e prec specifies accuracy of representation
   * as follows:
   * - prec = -5 (min), 100km
   * - prec = -3, 1km
   * - prec = 0, 1m
   * - prec = 3, 1mm
   * - prec = 6, 1um
   * - prec = 9 (max), 1nm
   **********************************************************************/
  std::string utm_ups_representation(int prec = 0) const;

  /**
   * Return MGRS string using alternative zone.  See MGRSRepresentation for
   * the interpretation of \e prec.
   **********************************************************************/
  std::string alt_mgrs_representation(int prec = 0) const;

  /**
   * Return string consisting of alternate UTM/UPS zone designator, easting,
   * and northing.  See UTMUPSRepresentation for the interpretation of \e
   * prec.
   **********************************************************************/
  std::string alt_utm_ups_representation(int prec = 0) const;

  /**
   * The major radius of the ellipsoid (meters).  This is the value for the
   * WGS84 ellipsoid because the UTM and UPS projections are based on this
   * ellipsoid.
   **********************************************************************/
  double major_radius() const;

  /**
   * The inverse flattening of the ellipsoid.  This is the value for the
   * WGS84 ellipsoid because the UTM and UPS projections are based on this
   * ellipsoid.
   **********************************************************************/
  double inverse_flattening() const;

  /**
   *Check whether or not the object has valid coordinates
   **********************************************************************/
  bool is_valid(void) const;

private:
  geo_coords_impl *impl_;
};

}  // End geographic namespace
}  // End vidtk namespace

#endif


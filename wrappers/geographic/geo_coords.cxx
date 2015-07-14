/*ckwg +5
 * Copyright 2010-2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <limits>
#include <GeographicLib/GeoCoords.hpp>
#include "geo_coords.h"

namespace vidtk
{
namespace geographic
{

bool
is_latlon_valid(const double lat, const double lon)
{
  // Copied from private method UTMUPS::CheckLatLon()
  bool is_valid = true;
  if (!(lat >= -90 && lat <= 90))
  {
    is_valid = false;
  }
  else if (!(lon >= -180 && lon <= 360))
  {
    is_valid = false;
  }

  return is_valid;
}


struct geo_coords_impl
{
  geo_coords_impl(void)
  : is_valid(false)
  { }

  bool is_valid;
  GeographicLib::GeoCoords c;
};


geo_coords
::geo_coords(void)
: impl_(0)
{
  impl_ = new geo_coords_impl;
}


geo_coords
::geo_coords(const std::string &s)
: impl_(0)
{
  impl_ = new geo_coords_impl;

  this->reset(s);
}


geo_coords
::geo_coords(double _latitude, double _longitude)
: impl_(0)
{
  impl_ = new geo_coords_impl;

  this->reset(_latitude, _longitude);
}


geo_coords
::geo_coords(int _zone, bool _is_north, double _easting, double _northing)
: impl_(0)
{
  impl_ = new geo_coords_impl;

  this->reset(_zone, _is_north, _easting, _northing);
}


// Copy constructor
geo_coords::
geo_coords (geo_coords const & obj)
{
  // do a deep copy
  this->impl_ = new geo_coords_impl();
  *this->impl_ = *obj.impl_;
}


// assignment operator
geo_coords & geo_coords::
operator= (geo_coords const & obj)
{
  if (this != &obj)
  {
    *this->impl_ = *obj.impl_;
  }

  return ( *this );
}


geo_coords::
~geo_coords()
{
  delete impl_;
  impl_ = 0;
}


bool
geo_coords
::reset(const std::string& s)
{
  try
  {
    this->impl_->c.Reset(s);
  }
  catch(const GeographicLib::GeographicErr &)
  {
    return this->impl_->is_valid = false;
  }
  return this->impl_->is_valid = true;
}


bool
geo_coords
::reset(double _latitude, double _longitude)
{
  try
  {
    this->impl_->c.Reset(_latitude, _longitude);
  }
  catch(const GeographicLib::GeographicErr &)
  {
    return this->impl_->is_valid = false;
  }
  return this->impl_->is_valid = true;
}


bool
geo_coords
::reset(int _zone, bool _is_north, double _easting, double _northing)
{
  try
  {
    this->impl_->c.Reset(_zone, _is_north, _easting, _northing);
  }
  catch(const GeographicLib::GeographicErr &)
  {
    return this->impl_->is_valid = false;
  }
  return this->impl_->is_valid = true;
}


double
geo_coords
::latitude(void) const
{
  return this->impl_->is_valid ? this->impl_->c.Latitude() :
                                std::numeric_limits<double>::max();
}


double
geo_coords
::longitude(void) const
{
  return this->impl_->is_valid ? this->impl_->c.Longitude() :
                                std::numeric_limits<double>::max();
}


double
geo_coords
::easting(void) const
{
  return this->impl_->is_valid ? this->impl_->c.Easting() :
                                std::numeric_limits<double>::max();
}


double
geo_coords
::northing(void) const
{
  return this->impl_->is_valid ? this->impl_->c.Northing() :
                                std::numeric_limits<double>::max();
}


double
geo_coords
::convergence(void) const
{
  return this->impl_->is_valid ? this->impl_->c.Convergence() :
                                std::numeric_limits<double>::max();
}


double
geo_coords
::scale(void) const
{
  return this->impl_->is_valid ? this->impl_->c.Scale() :
                                std::numeric_limits<double>::max();
}


bool
geo_coords
::is_north(void) const
{
  return this->impl_->is_valid ? this->impl_->c.Northp() : false;
}


char
geo_coords
::hemisphere(void) const
{
  return this->impl_->is_valid ? this->impl_->c.Hemisphere() : '\0';
}


int
geo_coords
::zone(void) const
{
  return this->impl_->is_valid ? this->impl_->c.Zone() :
                                std::numeric_limits<int>::max();
}


bool
geo_coords
::set_alt_zone(int _zone)
{
  try
  {
    this->impl_->c.SetAltZone(_zone);
  }
  catch(const GeographicLib::GeographicErr &)
  {
    return false;
  }
  return true;
}


int
geo_coords
::alt_zone(void) const
{
  return this->impl_->is_valid ? this->impl_->c.AltZone() :
                                std::numeric_limits<int>::max();
}


double
geo_coords
::alt_easting() const
{
  return this->impl_->is_valid ? this->impl_->c.AltEasting() :
                                std::numeric_limits<double>::max();
}


double
geo_coords
::alt_northing() const
{
  return this->impl_->is_valid ? this->impl_->c.AltNorthing() :
                                std::numeric_limits<double>::max();
}


double
geo_coords
::alt_convergence() const
{
  return this->impl_->is_valid ? this->impl_->c.AltConvergence() :
                                std::numeric_limits<double>::max();
}


double
geo_coords
::alt_scale() const
{
  return this->impl_->is_valid ? this->impl_->c.AltScale() :
                                std::numeric_limits<double>::max();
}


std::string
geo_coords
::geo_representation(int prec) const
{
  return this->impl_->is_valid ? this->impl_->c.GeoRepresentation(prec) : "";
}


std::string
geo_coords
::dms_representation(int prec) const
{
  return this->impl_->is_valid ? this->impl_->c.DMSRepresentation(prec) : "";
}


std::string
geo_coords
::mgrs_representation(int prec) const
{
  return this->impl_->is_valid ? this->impl_->c.MGRSRepresentation(prec) : "";
}

std::string
geo_coords
::utm_ups_representation(int prec) const
{
  return this->impl_->is_valid ? this->impl_->c.UTMUPSRepresentation(prec) : "";
}


std::string
geo_coords
::alt_mgrs_representation(int prec) const
{
  return this->impl_->is_valid ? this->impl_->c.AltMGRSRepresentation(prec) : "";
}


std::string
geo_coords
::alt_utm_ups_representation(int prec) const
{
  return this->impl_->is_valid ? this->impl_->c.AltUTMUPSRepresentation(prec):"";
}


double
geo_coords
::major_radius() const
{
  return this->impl_->c.MajorRadius();
}


double
geo_coords
::inverse_flattening() const
{
  return this->impl_->c.InverseFlattening();
}


bool
geo_coords
::is_valid(void) const
{
  return this->impl_->is_valid;
}

}
}


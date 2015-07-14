/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "GeodeticPosition.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::GeodeticPosition);

#include <logger/logger.h>
#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __STANAG_4676_GEODETICPOSITION__
VIDTK_LOGGER("STANAG_4676_GeodeticPosition");

namespace STANAG_4676
{

/// \brief Initialize
/// \param lat Provides an estimate of the latitude.
/// \param lon Provides an estimate of the longitude.
/// \param e Provides an estimate of the elevation.
GeodeticPosition
::GeodeticPosition(double lat, double lon, double e)
{
  this->setLatitude(lat);
  this->setLongitude(lon);
  this->setElevation(e);
}

/// \brief Provides an estimate of the latitude of an object or feature,
///        expressed as decimal degrees North (positive) or South (negative)
///        of the Equator.  WGS-84 datum.  Allowed values are -90 degrees to
///        90 degrees.
double
GeodeticPosition
::getLatitude(void) const
{
  return this->_latitude;
}

/// \copydoc getLatitude
void
GeodeticPosition
::setLatitude(double lat)
{
  if (lat > 90.0 || lat < -90.0)
  {
    LOG_WARN("Latitude is out of range.  Wrapping to within range.");
    lat = XMLIOBase::wrapToRange(lat, -90.0, 90.0, 180.0);
  }
  this->_latitude = lat;
}

/// \brief Provides an estimate of the longitude of an object or feature,
///        expressed as decimal degrees East (positive) or West (negative)
///        from the Prime Meridian. WGS-84 datum. Allowed values are -180
///        degrees to 180 degrees.
double
GeodeticPosition
::getLongitude(void) const
{
  return this->_longitude;
}

/// \copydoc getLongitude
void
GeodeticPosition
::setLongitude(double lon)
{
  if (lon > 180.0 || lon < -180.0)
  {
    LOG_WARN("Longitude is out of range.  Wrapping to withing range.");
    lon = XMLIOBase::wrapToRange(lon, -180.0, 180.0, 360.0);
  }
  this->_longitude = lon;
}

/// \brief Provides an estimate of the elevation of an object or feature,
///        expressed in meters (m) height above ellipsoid. WGS-84 datum.
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  GeodeticPosition,
  double,
  Elevation,
  _elevation)

TiXmlElement*
GeodeticPosition
::toXML(const std::string& name) const
{
  TiXmlElement* e = Position::toXML(name);
  e->SetAttribute("xsi:type", "GeodeticPosition");
  e->LinkEndChild(XMLIOBase::toXML("latitude", this->_latitude));
  e->LinkEndChild(XMLIOBase::toXML("longitude", this->_longitude));
  e->LinkEndChild(XMLIOBase::toXML("elevation", this->_elevation));
  return e;
}

GeodeticPosition
::GeodeticPosition(const TiXmlElement* elem)
  : Position(elem)
{
  double lat, lon;

  XMLIOBase::fromXML(elem, "latitude", lat);
  XMLIOBase::fromXML(elem, "longitude", lon);
  XMLIOBase::fromXML(elem, "elevation", this->_elevation, false);

  // Not set directly in order to reuse bounds checking
  this->setLatitude(lat);
  this->setLongitude(lon);
}

STANAG_4676_FROMXML(GeodeticPosition)

} // End STANAG_4676 namespace

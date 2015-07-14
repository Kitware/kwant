/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_GEODETICPOSITION_H_
#define VIDTK_LIBRARY_STANAG_4676_GEODETICPOSITION_H_

#include <stanag_4676/Position.h>

namespace STANAG_4676
{

/// \brief Provides an estimate of geodetic position, expressed in latitude
///        (decimal degrees), longitude (decimal degrees), and elevation
///        (meters - height above ellipsoid).  WGS-84 datum.
class GeodeticPosition : public Position
{
public:
  typedef vbl_smart_ptr<GeodeticPosition> sptr;

  GeodeticPosition(double lat, double lon, double e);

  virtual ~GeodeticPosition(void) { }

  STANAG_4676_IMPLEMENT_DOWNCAST(GeodeticPosition)

  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, Latitude)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, Longitude)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, Elevation)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  GeodeticPosition(const TiXmlElement* elem);

private:
  double _latitude;
  double _longitude;
  double _elevation;
};

} // End STANAG_4676 namespace

#endif

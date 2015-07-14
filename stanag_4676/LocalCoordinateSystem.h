/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_LOCALCOORDINATESYSTEM_H_
#define VIDTK_LIBRARY_STANAG_4676_LOCALCOORDINATESYSTEM_H_

#include <stanag_4676/XMLIOBase.h>
#include <stanag_4676/GeodeticPosition.h>

namespace STANAG_4676
{

/// \brief Provides parameters to define a local, Cartesian coordinate system.
///
/// Contains information about the origin of the local coordinate system
/// (expressed in Geodetic coordinates) and the orientation of the local system
/// with respect to the Earth Centered Earth Fixed (ECEF) reference frame.
/// This class is intended to accommodate tracking algorithms that compute in a
/// local coordinate system, (e.g. MOTION IMAGERY tracking in a planar pixel
/// space, radar GMTI tracking in a range-Doppler slant plane or its projected
/// ground plane).
class LocalCoordinateSystem : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<LocalCoordinateSystem> sptr;

  LocalCoordinateSystem(const GeodeticPosition::sptr origin, double x_rotation, double y_rotation, double z_rotation);

  virtual ~LocalCoordinateSystem(void) { }

  void setOrigin(GeodeticPosition::sptr origin);
  void setRotation(double x, double y, double z);

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  LocalCoordinateSystem(const TiXmlElement* elem);

private:
  GeodeticPosition::sptr _origin;
  double _x_rotation;
  double _y_rotation;
  double _z_rotation;
};

} // End STANAG_4676 namespace

#endif

/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "LocalCoordinateSystem.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::LocalCoordinateSystem);

#include <logger/logger.h>
#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __STANAG_4676_LOCALCOORDINATESYSTEM__
VIDTK_LOGGER("STANAG_4676_LocalCoordinateSystem");

namespace STANAG_4676
{

/// \brief Initialize
LocalCoordinateSystem
::LocalCoordinateSystem(const GeodeticPosition::sptr origin,
                        double x_rotation, double y_rotation, double z_rotation)
  : _origin(origin)
{
  this->setRotation(x_rotation, y_rotation, z_rotation);
}

/// \brief Provides the origin of a Local Coordinate System
void
LocalCoordinateSystem
::setOrigin(GeodeticPosition::sptr origin)
{
  this->_origin = origin;
}

/// \brief Provides the angle of rotation about the x, y, and z axis between
///        a local coordinate frame and the ECEF reference frame, expressed
///        in decimal degrees. Values range between 0 and 360 degrees.  The
///        x, y, and z axis rotations must be applied in order.
void
LocalCoordinateSystem
::setRotation(double x, double y, double z)
{
  if (x < 0.0 || x > 360.0)
  {
    LOG_WARN("x-axis rotation is out of range.  Wrapping to range");
    x = XMLIOBase::wrapToRange(x, 0.0, 360.0, 360.0);
  }
  if (y < 0.0 || y > 360.0)
  {
    LOG_WARN("y-axis rotation is out of range.  Wrapping to range");
    y = XMLIOBase::wrapToRange(y, 0.0, 360.0, 360.0);
  }
  if (z < 0.0 || z > 360.0)
  {
    LOG_WARN("z-axis rotation is out of range.  Wrapping to range");
    z = XMLIOBase::wrapToRange(z, 0.0, 360.0, 360.0);
  }
  this->_x_rotation = x;
  this->_y_rotation = y;
  this->_z_rotation = z;
}

TiXmlElement*
LocalCoordinateSystem
::toXML(const std::string& name) const
{
  TiXmlElement* e = new TiXmlElement(name);
  e->SetAttribute("xsi:type", "LocalCoordinateSystem");
  e->LinkEndChild(this->_origin->toXML("origin"));
  e->LinkEndChild(XMLIOBase::toXML("x-axisRotation", this->_x_rotation));
  e->LinkEndChild(XMLIOBase::toXML("y-axisRotation", this->_y_rotation));
  e->LinkEndChild(XMLIOBase::toXML("z-axisRotation", this->_z_rotation));
  return e;
}

LocalCoordinateSystem
::LocalCoordinateSystem(const TiXmlElement* elem)
{
  double x, y, z;

  XMLIOBase::fromXML<GeodeticPosition>(elem, "origin", this->_origin);
  XMLIOBase::fromXML(elem, "x-axisRotation", x);
  XMLIOBase::fromXML(elem, "y-axisRotation", y);
  XMLIOBase::fromXML(elem, "z-axisRotation", z);

  // Not set directly in order to reuse bounds checking
  this->setRotation(x, y, z);
}

STANAG_4676_FROMXML(LocalCoordinateSystem)

} // End STANAG_4676 namespace

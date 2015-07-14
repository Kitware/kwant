/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "LocalCartesianVelocity.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::LocalCartesianVelocity);

namespace STANAG_4676
{

/// \brief Initialize
LocalCartesianVelocity
::LocalCartesianVelocity(double velx, double vely, double velz)
  : _velx(velx), _vely(vely), _velz(velz)
{
}

/// \brief Provides an estimate of the x-component of velocity, expressed in
///        meters per second (m/s).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  LocalCartesianVelocity,
  double,
  VelX,
  _velx)

/// \brief Provides an estimate of the y-component of velocity, expressed in
///        meters per second (m/s).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  LocalCartesianVelocity,
  double,
  VelY,
  _vely)

/// \brief Provides an estimate of the z-component of velocity, expressed in
///        meters per second (m/s).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  LocalCartesianVelocity,
  double,
  VelZ,
  _velz)

TiXmlElement*
LocalCartesianVelocity
::toXML(const std::string& name) const
{
  TiXmlElement* e = Velocity::toXML(name);
  e->SetAttribute("xsi:type", "LocalCartesianVelocity");
  e->LinkEndChild(XMLIOBase::toXML("velx", this->_velx));
  e->LinkEndChild(XMLIOBase::toXML("vely", this->_vely));
  e->LinkEndChild(XMLIOBase::toXML("velz", this->_velz));
  return e;
}

LocalCartesianVelocity
::LocalCartesianVelocity(const TiXmlElement* elem)
  : Velocity(elem)
{
  XMLIOBase::fromXML(elem, "velx", this->_velx);
  XMLIOBase::fromXML(elem, "vely", this->_vely);
  XMLIOBase::fromXML(elem, "velz", this->_velz);
}

STANAG_4676_FROMXML(LocalCartesianVelocity)

} // End STANAG_4676 namespace

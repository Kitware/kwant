/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "LocalCartesianAcceleration.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::LocalCartesianAcceleration);

namespace STANAG_4676
{

/// \brief Initialize
LocalCartesianAcceleration
::LocalCartesianAcceleration(double accx, double accy, double accz)
  : _accx(accx), _accy(accy), _accz(accz)
{
}

/// \brief Provides the x-component of acceleration, expressed in meters per
///        second squared (m/s<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  LocalCartesianAcceleration,
  double,
  AccX,
  _accx)

/// \brief Provides the y-component of acceleration, expressed in meters per
///        second squared (m/s<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  LocalCartesianAcceleration,
  double,
  AccY,
  _accy)

/// \brief Provides the z-component of acceleration, expressed in meters per
///        second squared (m/s<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  LocalCartesianAcceleration,
  double,
  AccZ,
  _accz)

TiXmlElement*
LocalCartesianAcceleration
::toXML(const std::string& name) const
{
  TiXmlElement* e = Acceleration::toXML(name);
  e->SetAttribute("xsi:type", "LocalCartesianAcceleration");
  e->LinkEndChild(XMLIOBase::toXML("accx", this->_accx));
  e->LinkEndChild(XMLIOBase::toXML("accy", this->_accy));
  e->LinkEndChild(XMLIOBase::toXML("accz", this->_accz));
  return e;
}

LocalCartesianAcceleration
::LocalCartesianAcceleration(const TiXmlElement* elem)
  : Acceleration(elem)
{
  XMLIOBase::fromXML(elem, "accx", this->_accx);
  XMLIOBase::fromXML(elem, "accy", this->_accy);
  XMLIOBase::fromXML(elem, "accz", this->_accz);
}

STANAG_4676_FROMXML(LocalCartesianAcceleration)

} // End STANAG_4676 namespace

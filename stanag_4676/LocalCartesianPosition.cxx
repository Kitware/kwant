/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "LocalCartesianPosition.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::LocalCartesianPosition);

namespace STANAG_4676
{

/// \brief Initialize
LocalCartesianPosition
::LocalCartesianPosition(double posx, double posy, double posz,
                         const LocalCoordinateSystem::sptr& localSystem)
  : _posx(posx), _posy(posy), _posz(posz), _localSystem(localSystem)
{
}

/// \brief Provides an estimate of the x-component of position, expressed in
///        a Local Coordinate System, unit of meters (m).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  LocalCartesianPosition,
  double,
  PosX,
  _posx)

/// \brief Provides an estimate of the y-component of position, expressed in
///        a Local Coordinate System, unit of meters (m).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  LocalCartesianPosition,
  double,
  PosY,
  _posy)

/// \brief Provides an estimate of the z-component of position, expressed in
///        a Local Coordinate System, unit of meters (m).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  LocalCartesianPosition,
  double,
  PosZ,
  _posz)

/// \brief Provides parameters to define a local coordinate system, in terms
///        of its translation and rotation with respect to the Earth Centered
///        Earth Fixed (ECEF) reference frame.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  LocalCartesianPosition,
  LocalCoordinateSystem::sptr,
  LocalSystem,
  _localSystem)

TiXmlElement*
LocalCartesianPosition
::toXML(const std::string& name) const
{
  TiXmlElement* e = Position::toXML(name);
  e->SetAttribute("xsi:type", "LocalCartesianPosition");
  e->LinkEndChild(XMLIOBase::toXML("posx", this->_posx));
  e->LinkEndChild(XMLIOBase::toXML("posy", this->_posy));
  e->LinkEndChild(XMLIOBase::toXML("posz", this->_posz));
  e->LinkEndChild(this->_localSystem->toXML("localSystem"));
  return e;
}

LocalCartesianPosition
::LocalCartesianPosition(const TiXmlElement* elem)
  : Position(elem)
{
  XMLIOBase::fromXML<LocalCoordinateSystem>(elem, "localSystem", this->_localSystem);
  XMLIOBase::fromXML(elem, "posx", this->_posx);
  XMLIOBase::fromXML(elem, "posy", this->_posy);
  XMLIOBase::fromXML(elem, "posz", this->_posz, false);
}

STANAG_4676_FROMXML(LocalCartesianPosition)

} // End STANAG_4676 namespace

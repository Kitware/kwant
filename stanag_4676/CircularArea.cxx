/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "CircularArea.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::CircularArea);

namespace STANAG_4676
{

/// \brief Initialize
CircularArea
::CircularArea(Position::sptr center, double radius)
  : _center(center), _radius(radius)
{
}

/// \brief Provides the center point of a defined circular area.  Used in
///        combination with a radius measurement to specify a circular area.
///
/// In the case of TrackProductionArea, the center point should be reported in
/// geodetic coordinates (in the GeodeticPosition class), latitude and
/// longitude only - no report for elevation. In the case of ObjectMask, the
/// center point should be reported in local Cartesian coordinates (in the
/// LocalCartesianPosition class), x and y components only - no report for
/// z-component.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  CircularArea,
  Position::sptr,
  Center,
  _center)

/// \brief Provides the radius of a defined circular area, expressed in
///        meters (m). Used in combination with a center point to specify a
///        circular area.
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CircularArea,
  double,
  Radius,
  _radius)

TiXmlElement*
CircularArea
::toXML(const std::string& name) const
{
  TiXmlElement* e = Area::toXML(name);
  e->SetAttribute("xsi:type", "CircularArea");
  e->LinkEndChild(this->_center->toXML("center"));
  e->LinkEndChild(XMLIOBase::toXML("radius", this->_radius));
  return e;
}

CircularArea
::CircularArea(const TiXmlElement* elem)
  : Area(elem)
{
  XMLIOBase::fromXML<Position>(elem, "center", this->_center);
  XMLIOBase::fromXML(elem, "radius", this->_radius);
}

STANAG_4676_FROMXML(CircularArea)

} // End STANAG_4676 namespace

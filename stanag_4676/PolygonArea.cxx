/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "PolygonArea.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::PolygonArea);

#include <logger/logger.h>
#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __STANAG_4676_POLYGONAREA__
VIDTK_LOGGER("STANAG_4676_PolygonArea");

namespace STANAG_4676
{

/// \brief Initialize
PolygonArea
::PolygonArea(void)
{
}

/// \brief Provides a of 3 or more vertices that define a polygonal area.
///
/// In the case of TrackProductionArea, the vertices should be reported in
/// geodetic coordinates (in the GeodeticPosition class), latitude and
/// longitude only - no report for altitude. In the case of ObjectMask, the
/// vertices should be reported in local Cartesian coordinates (in the
/// LocalCartesianPosition class), x and y components only - no report for
/// z-component.  Boundary points must be given in sequential order, and in a
/// clockwise direction, to eliminate ambiguities.  The starting/end point
/// shall be listed only once.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  PolygonArea,
  std::vector<Position::sptr>,
  Points,
  _boundaryPoints)

/// \brief Convenience method to add a point.
void
PolygonArea
::addPoint(const Position::sptr& p)
{
  this->_boundaryPoints.push_back(p);
}

TiXmlElement*
PolygonArea
::toXML(const std::string& name) const
{
  if (this->_boundaryPoints.size() < 3)
  {
    LOG_ERROR("Not enough boundary points; at least 3 are needed.");
    return NULL;
  }
  TiXmlElement* e = Area::toXML(name);
  e->SetAttribute("xsi:type", "PolygonArea");
  for (size_t i = 0; i < this->_boundaryPoints.size(); ++i)
  {
    e->LinkEndChild(this->_boundaryPoints[i]->toXML("areaBoundaryPoints"));
  }
  return e;
}

PolygonArea
::PolygonArea(const TiXmlElement* elem)
  : Area(elem)
{
  const TiXmlElement* child_elem = 0;
  const std::string point_name = "areaBoundaryPoints";
  while ((child_elem = XMLIOBase::nextChild(elem, point_name, child_elem)))
  {
    const Position::sptr p =
      Position::fromXML(child_elem, &this->_error_count);

    if (p)
    {
      this->addPoint(p);
      continue;
    }

    ++this->_error_count;
  }

  const size_t np = this->_boundaryPoints.size();
  if (np < 3)
    throw MissingChildException(elem, point_name, 3, np);
}

STANAG_4676_FROMXML(PolygonArea)

} // End STANAG_4676 namespace

/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "TrackPoint.h"
#include <vbl/vbl_smart_ptr.txx>
#include <limits>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::TrackPoint);

namespace STANAG_4676
{

TrackPoint
::TrackPoint(const boost::uuids::uuid& uuid, Security::sptr security,
             const boost::posix_time::ptime& time, GeodeticPosition::sptr pos)
  : TrackItem(uuid, security, time), _position(pos),
    _speed(std::numeric_limits<int>::min()),
    _course(std::numeric_limits<double>::min()),
    _type(TrackPointType_UNSET), _pointSource(ModalityType_UNSET),
    _objectMask() //, _detail(NULL)
{
}

/// \brief Provides the position of an object being tracked.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackPoint,
  GeodeticPosition::sptr,
  Position,
  _position)

/// \brief Provides the speed of an object being tracked, expressed in
///        meters per second (m/s).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  TrackPoint,
  int,
  Speed,
  _speed)

/// \brief Provides the course of an object being tracked, expressed in
///        decimal degrees and measured from true north in a clockwise
///        direction.
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  TrackPoint,
  double,
  Course,
  _course)

/// \brief Provides information of whether a track point is estimated, or
///        predicted manually or automatically.
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  TrackPoint,
  TrackPointType,
  Type,
  _type)

/// \brief Provides information related to the source of the track point data
///        (i.e. RADAR, MOTION IMAGERY, ESM).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  TrackPoint,
  ModalityType,
  PointSource,
  _pointSource)

/// \brief Provides a spatial outline of an object being tracked.
///
/// For example, in case of MOTION IMAGERY tracking, a box or polygon
/// surrounding the object may be specified.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackPoint,
  Area::sptr,
  ObjectMask,
  _objectMask)

/// \brief Provides detailed information related to a track point.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackPoint,
  TrackPointDetail::sptr,
  Detail,
  _detail)

TiXmlElement*
TrackPoint
::toXML(const std::string& name) const
{
  TiXmlElement* e = TrackItem::toXML(name);
  e->SetAttribute("xsi:type", "TrackPoint");
  e->LinkEndChild(this->_position->toXML("trackPointPosition"));
  if (this->_speed != std::numeric_limits<int>::min())
  {
    e->LinkEndChild(XMLIOBase::toXML("trackPointSpeed", this->_speed));
  }
  if (this->_course != std::numeric_limits<double>::min())
  {
    e->LinkEndChild(XMLIOBase::toXML("trackPointCourse", this->_course));
  }
  if (this->_type != TrackPointType_UNSET)
  {
    e->LinkEndChild(XMLIOBase::toXML("trackPointType", this->_type));
  }
  if (this->_pointSource != ModalityType_UNSET)
  {
    e->LinkEndChild(XMLIOBase::toXML("trackPointSource", this->_pointSource));
  }
  if (this->_objectMask)
  {
    e->LinkEndChild(this->_objectMask->toXML("trackPointObjectMask"));
  }
  if (this->_detail)
  {
    e->LinkEndChild(this->_detail->toXML("TrackPointDetail"));
  }
  return e;
}

TrackPoint
::TrackPoint(const TiXmlElement* elem)
  : TrackItem(elem)
{
  XMLIOBase::fromXML<GeodeticPosition>(elem, "trackPointPosition",
                                       this->_position);
  XMLIOBase::fromXML(elem, "trackPointSpeed", this->_speed, false);
  XMLIOBase::fromXML(elem, "trackPointCourse", this->_course, false);
  XMLIOBase::fromXML(elem, "trackPointType", this->_type, false);
  XMLIOBase::fromXML(elem, "trackPointSource", this->_pointSource, false);
  XMLIOBase::fromXML<Area>(elem, "trackPointObjectMask",
                           this->_objectMask, false);
  XMLIOBase::fromXML<TrackPointDetail>(elem, "TrackPointDetail",
                                       this->_detail, false);
}

STANAG_4676_FROMXML(TrackPoint)

} // End STANAG_4676 namespace

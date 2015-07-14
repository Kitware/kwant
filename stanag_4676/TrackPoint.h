/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_TRACKPOINT_H_
#define VIDTK_LIBRARY_STANAG_4676_TRACKPOINT_H_

#include <stanag_4676/TrackItem.h>
#include <stanag_4676/Enums.h>
#include <stanag_4676/GeodeticPosition.h>
#include <stanag_4676/Area.h>
#include <stanag_4676/TrackPointDetail.h>

namespace STANAG_4676
{

/// \brief Provides information about a track point.
class TrackPoint : public TrackItem
{
public:
  typedef vbl_smart_ptr<TrackPoint> sptr;

  TrackPoint(const boost::uuids::uuid& uuid, Security::sptr security,
             const boost::posix_time::ptime& time, GeodeticPosition::sptr pos);

  virtual ~TrackPoint(void) { }

  STANAG_4676_IMPLEMENT_DOWNCAST(TrackPoint)

  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(GeodeticPosition::sptr, Position)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(int, Speed)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, Course)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(TrackPointType, Type)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(ModalityType, PointSource)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(Area::sptr, ObjectMask)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(TrackPointDetail::sptr, Detail)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  TrackPoint(const TiXmlElement* elem);

private:
  GeodeticPosition::sptr _position;
  int _speed;
  double _course;
  TrackPointType _type;
  ModalityType _pointSource;
  Area::sptr _objectMask;
  TrackPointDetail::sptr _detail;
};

} // End STANAG_4676 namespace

#endif

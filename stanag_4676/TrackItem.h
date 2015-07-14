/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_TRACKITEM_H_
#define VIDTK_LIBRARY_STANAG_4676_TRACKITEM_H_

#include <stanag_4676/XMLIOBase.h>
#include <stanag_4676/Security.h>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/uuid/uuid.hpp>

namespace STANAG_4676
{

class TrackPoint;

/// \brief Provides information either about a track point or about amplifying,
///        non-kinematic parameters/features pertinent to a track.
///
/// Every track can hold multiple track items. One track item describes a
/// single, timed event of the track. The TrackItem is abstract and has two
/// specializations:
///   1. TrackPoint; the most common, frequently used type to store position
///      information
///   2. TrackInformation; containing track information like identity, etc.
class TrackItem : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<TrackItem> sptr;

  TrackItem(const boost::uuids::uuid& uuid, Security::sptr security,
            const boost::posix_time::ptime& time);

  virtual ~TrackItem(void) = 0;

  STANAG_4676_DECLARE_DOWNCAST(TrackPoint)

  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(boost::uuids::uuid, UUID)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(Security::sptr, Security)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(boost::posix_time::ptime, Time)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::string, Source)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::string, Comment)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  TrackItem(const TiXmlElement* elem);

  boost::uuids::uuid _uuid;
  Security::sptr _security;
  boost::posix_time::ptime _time;
  std::string _source;
  std::string _comment;
};
inline TrackItem::~TrackItem(void) { }

} // End STANAG_4676 namespace

#endif

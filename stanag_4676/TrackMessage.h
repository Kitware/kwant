/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_TRACKMESSAGE_H_
#define VIDTK_LIBRARY_STANAG_4676_TRACKMESSAGE_H_

#include <boost/date_time.hpp>
#include <stanag_4676/XMLIOBase.h>
#include <stanag_4676/Security.h>
#include <stanag_4676/DataSource.h>
#include <stanag_4676/TrackSource.h>
#include <stanag_4676/IDdata.h>
#include <stanag_4676/Track.h>

namespace STANAG_4676
{

/// \brief Provides top-level information about a track message.
class TrackMessage : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<TrackMessage> sptr;

  TrackMessage(Security::sptr security,
               const boost::posix_time::ptime& createdTime,
               DataSource::sptr dataSource, TrackSource::sptr trackSource,
               IDdata::sptr senderId);

  virtual ~TrackMessage(void) { }

  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(Security::sptr, Security)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(boost::posix_time::ptime, CreatedTime)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(DataSource::sptr, DataSource)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(TrackSource::sptr, TrackSource)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(IDdata::sptr, SenderID)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::vector<Track::sptr>, Tracks)

  void addTrack(Track::sptr t);

  virtual TiXmlElement* toXML(const std::string& name) const;
  TiXmlElement* toXML(void) const { return this->toXML("TrackMessage"); }

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  TrackMessage(const TiXmlElement* elem);

private:
  std::string _stanagVersion;
  Security::sptr _security;
  boost::posix_time::ptime _createdTime;
  DataSource::sptr _dataSource;
  TrackSource::sptr _trackSource;
  IDdata::sptr _senderId;
  std::vector<Track::sptr> _tracks;
};

} // End STANAG_4676 namespace

#endif

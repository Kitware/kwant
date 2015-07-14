/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "TrackMessage.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::TrackMessage);

namespace STANAG_4676
{

/// \brief Initialize
TrackMessage
::TrackMessage(Security::sptr security,
               const boost::posix_time::ptime& createdTime,
               DataSource::sptr dataSource, TrackSource::sptr trackSource,
               IDdata::sptr senderId)
  : _stanagVersion("3.1"), _security(security), _createdTime(createdTime),
    _dataSource(dataSource), _trackSource(trackSource), _senderId(senderId)
{
}

/// \brief Provides security and dissemination/control/releasibility
///        information applicable at the Track Message level, in accordance
///        with [EAPC(AC/322-SC/5)N(2006)0008].
///
/// Used to facilitate the exchange of information through Information Exchange
/// Gateways [NC3TA].  A TrackMessage can contain multiple Tracks and each
/// track contain multiple Track Items, each of these having its own security
/// classification. This construction allows users to extract and disseminate
/// information having a lower security classification level than a
/// TrackMessage. For example, a TrackMessage may contain tracks derived from
/// Friendly Force Information which are classified as SECRET, as well as
/// non-cooperative tracks that are UNCLASSIFIED. The message security level
/// shall not be lower than the highest security classification of any
/// contained track items.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackMessage,
  Security::sptr,
  Security,
  _security)

/// \brief Provides the date and time of the moment a message was created, in
///        Coordinated Universal Time (UTC) reference frame.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackMessage,
  boost::posix_time::ptime,
  CreatedTime,
  _createdTime)

/// \brief Provides information about the source of data used to create the
///        SPADE track message.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackMessage,
  DataSource::sptr,
  DataSource,
  _dataSource)

/// \brief Provides information about the tracker used to create the SPADE
///        track message.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackMessage,
  TrackSource::sptr,
  TrackSource,
  _trackSource)

/// \brief Provides identification information about the originator of a
///        TrackMessage.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackMessage,
  IDdata::sptr,
  SenderID,
  _senderId)

/// \brief Provides a list of tracks in the current message.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackMessage,
  std::vector<Track::sptr>,
  Tracks,
  _tracks)

/// \brief Add a track to the current message.
void
TrackMessage
::addTrack(Track::sptr t)
{
  this->_tracks.push_back(t);
}

TiXmlElement*
TrackMessage
::toXML(const std::string& name) const
{
  TiXmlElement* e = new TiXmlElement(name);
  e->SetAttribute("xsi:type", "TrackMessage");
  e->LinkEndChild(XMLIOBase::toXML("stanagVersion", this->_stanagVersion));
  e->LinkEndChild(this->_security->toXML("messageSecurity"));
  e->LinkEndChild(XMLIOBase::toXML("msgCreatedTime", this->_createdTime));
  e->LinkEndChild(this->_dataSource->toXML("dataSource"));
  e->LinkEndChild(this->_trackSource->toXML("trackSource"));
  e->LinkEndChild(this->_senderId->toXML("senderId"));
  for (size_t i = 0; i < this->_tracks.size(); ++i)
  {
    e->LinkEndChild(this->_tracks[i]->toXML("tracks"));
  }
  return e;
}

TrackMessage
::TrackMessage(const TiXmlElement* elem)
{
  // Read basic attributes
  XMLIOBase::fromXML(elem, "stanagVersion", this->_stanagVersion);
  XMLIOBase::fromXML<Security>(elem, "messageSecurity", this->_security);
  XMLIOBase::fromXML(elem, "msgCreatedTime", this->_createdTime);
  XMLIOBase::fromXML<DataSource>(elem, "dataSource", this->_dataSource);
  XMLIOBase::fromXML<TrackSource>(elem, "trackSource", this->_trackSource);
  XMLIOBase::fromXML<IDdata>(elem, "senderId", this->_senderId);

  // Read tracks
  const TiXmlElement* child_elem = 0;
  const std::string track_name = "tracks";
  while ((child_elem = XMLIOBase::nextChild(elem, track_name, child_elem)))
  {
    const Track::sptr p = Track::fromXML(child_elem, &this->_error_count);

    if (p)
    {
      this->addTrack(p);
      continue;
    }

    ++this->_error_count;
  }

  // At least one track is required
  const size_t np = this->_tracks.size();
  if (np < 1)
    throw MissingChildException(elem, track_name, 1, np);
}

STANAG_4676_FROMXML(TrackMessage)

} // End STANAG_4676 namespace

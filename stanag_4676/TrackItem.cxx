/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "TrackItem.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::TrackItem);

#include "TrackPoint.h"

namespace STANAG_4676
{

/// \brief Initialize
TrackItem
::TrackItem(const boost::uuids::uuid& uuid, Security::sptr security,
            const boost::posix_time::ptime& time)
  : _uuid(uuid), _security(security), _time(time), _source(""), _comment("")
{
}

/// \brief Provides a 32-character hexidecimal  Universal Unique
///        Identification (UUID), standardized by the Open Software
///        Foundation, of a TrackItem. Reference ISO/IEC 9834-8:2005.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackItem,
  boost::uuids::uuid,
  UUID,
  _uuid)

/// \brief Provides security and dissemination/control/releasibility
///        information applicable at the Track Item level.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackItem,
  Security::sptr,
  Security,
  _security)

/// \brief Provides the time of the event (track item)
///
/// For example, indicating the time when the tracked target was on a specific
/// position. Or indicating on what time the tracked target was identified as
/// FRIENDLY.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackItem,
  boost::posix_time::ptime,
  Time,
  _time)

/// \brief Provides information related to the source data for a reported
///        track item.
///
/// Examples include: URI, or UUIDs
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackItem,
  std::string,
  Source,
  _source)

/// \brief Provides a free text field to transmit information related to a
///        track item.
///
/// Examples of usage include commenting on identity, attaching comment(s) to
/// MOTION IMAGERY track point.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackItem,
  std::string,
  Comment,
  _comment)

TiXmlElement*
TrackItem
::toXML(const std::string& name) const
{
  TiXmlElement* e = new TiXmlElement(name);
  e->LinkEndChild(XMLIOBase::toXML("trackItemUUID", this->_uuid));
  e->LinkEndChild(this->_security->toXML("trackItemSecurity"));
  e->LinkEndChild(XMLIOBase::toXML("trackItemTime", this->_time));
  if (!this->_source.empty())
  {
    e->LinkEndChild(XMLIOBase::toXML("trackItemSource", this->_source));
  }
  if (!this->_comment.empty())
  {
    e->LinkEndChild(XMLIOBase::toXML("trackItemComment", this->_comment));
  }
  return e;
}

TrackItem::sptr
TrackItem
::fromXML(const TiXmlElement* elem, int* error_count)
{
  const std::string* const type = elem->Attribute(std::string("xsi:type"));
  if (type)
  {
    STANAG_4676_FROMXML_DOWNCAST(elem, type, TrackPoint)

    XMLIOBase::typeError(error_count, elem, *type);
    return sptr();
  }
  XMLIOBase::typeError(error_count, elem);
  return sptr();
}

TrackItem
::TrackItem(const TiXmlElement* elem)
{
  XMLIOBase::fromXML(elem, "trackItemUUID", this->_uuid);
  XMLIOBase::fromXML<Security>(elem, "trackItemSecurity", this->_security);
  XMLIOBase::fromXML(elem, "trackItemTime", this->_time);
  XMLIOBase::fromXML(elem, "trackItemSource", this->_source, false);
  XMLIOBase::fromXML(elem, "trackItemComment", this->_comment, false);
}

} // End STANAG_4676 namespace

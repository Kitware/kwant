/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "Track.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::Track);

namespace STANAG_4676
{

/// \brief Initialize
Track
::Track(const boost::uuids::uuid& uuid, const std::string& number,
        Security::sptr security, const ExerciseIndicator& e,
        const SimulationIndicator& s)
  : _uuid(uuid), _number(number), _status(TrackStatus_UNSET), _security(security),
    _comment(""), _exerciseIndicator(e), _simulationIndicator(s)
{
}

/// \brief Provides a 32-character hexidecimal Universal Unique Identification
///        (UUID), standardized by the Open Software Foundation, of a Track.
///        Reference ISO/IEC 9834-8:2005.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  Track,
  boost::uuids::uuid,
  UUID,
  _uuid)

/// \brief Provides Track Number, in accordance with ADatP-33(A).
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  Track,
  std::string,
  Number,
  _number)

/// \brief Provides information related to the status of a track.
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  Track,
  TrackStatus,
  Status,
  _status)

/// \brief Provides security and dissemination/control/releasibility
///        information applicable at the Track level, in accordance with
///        [EAPC(AC/322-SC/5)N(2006)0008].
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  Track,
  Security::sptr,
  Security,
  _security)

/// \brief Provides a free text field to express amplified information
///        regarding the track, such as object identity, recognized
///        activity/behavior, etc.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  Track,
  std::string,
  Comment,
  _comment)

/// \brief Provides an indication of whether a track is generated from
///        operational, exercise, or test data.
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  Track,
  ExerciseIndicator,
  ExerciseIndicator,
  _exerciseIndicator)

/// \brief Provides an indication of whether a track is generated from real,
///        simulated, or synthesized data.
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  Track,
  SimulationIndicator,
  SimulationIndicator,
  _simulationIndicator)

/// \brief Provides a list of items in the current track.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  Track,
  std::vector<TrackItem::sptr>,
  Items,
  _items)

/// \brief Add an item to the current track.
void
Track
::addItem(TrackItem::sptr item)
{
  this->_items.push_back(item);
}

TiXmlElement*
Track
::toXML(const std::string& name) const
{
  TiXmlElement* e = new TiXmlElement(name);
  e->SetAttribute("xsi:type", "Track");
  e->LinkEndChild(XMLIOBase::toXML("trackUUID", this->_uuid));
  e->LinkEndChild(XMLIOBase::toXML("trackNumber", this->_number));
  if (this->_status != TrackStatus_UNSET)
  {
    e->LinkEndChild(XMLIOBase::toXML("trackStatus", this->_status));
  }
  e->LinkEndChild(this->_security->toXML("trackSecurity"));
  if (!this->_comment.empty())
  {
    e->LinkEndChild(XMLIOBase::toXML("trackComment", this->_comment));
  }
  e->LinkEndChild(XMLIOBase::toXML("exerciseIndicator", this->_exerciseIndicator));
  e->LinkEndChild(XMLIOBase::toXML("simulationIndicator", this->_simulationIndicator));
  for (size_t i = 0; i < this->_items.size(); ++i)
  {
    e->LinkEndChild(this->_items[i]->toXML("items"));
  }
  return e;
}

Track
::Track(const TiXmlElement* elem)
{
  XMLIOBase::fromXML(elem, "trackUUID", this->_uuid);
  XMLIOBase::fromXML(elem, "trackNumber", this->_number);
  XMLIOBase::fromXML(elem, "trackStatus", this->_status, false);
  XMLIOBase::fromXML<Security>(elem, "trackSecurity", this->_security);
  XMLIOBase::fromXML(elem, "trackComment", this->_comment, false);
  XMLIOBase::fromXML(elem, "exerciseIndicator", this->_exerciseIndicator);
  XMLIOBase::fromXML(elem, "simulationIndicator", this->_simulationIndicator);

  const TiXmlElement* child_elem = 0;
  const std::string item_name = "items";
  while ((child_elem = XMLIOBase::nextChild(elem, item_name, child_elem)))
  {
    const TrackItem::sptr p =
      TrackItem::fromXML(child_elem, &this->_error_count);

    if (p)
    {
      this->addItem(p);
      continue;
    }

    ++this->_error_count;
  }
}

STANAG_4676_FROMXML(Track)

} // End STANAG_4676 namespace

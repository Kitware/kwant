/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "TrackProductType.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::TrackProductType);

namespace STANAG_4676
{

/// \brief Initialize
TrackProductType
::TrackProductType(TrackMessage::sptr msg)
  : _message(msg)
{
}

/// \brief This contains the message payload, which can be one of any
///        type of message.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackProductType,
  TrackMessage::sptr,
  Message,
  _message)

TiXmlElement*
TrackProductType
::toXML(const std::string& name) const
{
  TiXmlElement* e = new TiXmlElement(name);
  e->SetAttribute("xmlns", "urn:stanag4676.rya.afrl.afmc.af.mil:spade.icd.stanag4676");
  e->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  e->SetAttribute("xsi:type", "TrackProductType");
  e->LinkEndChild(this->_message->toXML("trackMessage"));
  return e;
}

TrackProductType
::TrackProductType(const TiXmlElement* elem)
{
  XMLIOBase::fromXML<TrackMessage>(elem, "trackMessage", this->_message);
}

STANAG_4676_FROMXML(TrackProductType)

} // End STANAG_4676 namespace

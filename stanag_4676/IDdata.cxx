/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "IDdata.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::IDdata);

namespace STANAG_4676
{

/// \brief Initialize
IDdata
::IDdata(const std::string& id, const std::string& nationality)
  : _id(id), _nationality(nationality)
{
}

/// \brief Provides a unique station identification number/designator of a
///        4676 capable system.
///
/// The stationID is determined by the nation owning the platform, whose
/// responsibility is to ensure that all its platforms are uniquely identified
/// within the set of platforms it owns.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  IDdata,
  std::string,
  Id,
  _id)

/// \brief Provides the nationality of a STANAG 4676 capable system.
///
/// String of alphanumeric characters denoting 3-letter country codes, in
/// accordance with STANAG 1059.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  IDdata,
  std::string,
  Nationality,
  _nationality)

TiXmlElement*
IDdata
::toXML(const std::string& name) const
{
  TiXmlElement* e = new TiXmlElement(name);
  e->SetAttribute("xsi:type", "IDdata");
  e->LinkEndChild(XMLIOBase::toXML("stationID", this->_id));
  e->LinkEndChild(XMLIOBase::toXML("nationality", this->_nationality));
  return e;
}

IDdata
::IDdata(const TiXmlElement* elem)
{
  XMLIOBase::fromXML(elem, "stationID", this->_id);
  XMLIOBase::fromXML(elem, "nationality", this->_nationality);
}

STANAG_4676_FROMXML(IDdata)

} // End STANAG_4676 namespace

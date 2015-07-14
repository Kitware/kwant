/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "TrackSource.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::TrackSource);

#include <logger/logger.h>
#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __STANAG_4676_TRACKSOURCE__
VIDTK_LOGGER("STANAG_4676_TrackSource");

namespace STANAG_4676
{

/// \brief Initialize
TrackSource
::TrackSource(const std::string& name, const std::string& desc)
  : _name(name)
{
  this->setDescription(desc);
}

/// \brief Provides the unique name of the tracker.  This is used to
///        determine where tracks were derived from.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackSource,
  std::string,
  Name,
  _name)

/// \brief Provides a description of the tracker.  This could be used to
///        describe the tracker, its algorithm, or any other information
///        about the tracker.  Limit to 4,000 characters.
std::string
TrackSource
::getDescription(void) const
{
  return this->_description;
}

/// \copydoc getDescription
void
TrackSource
::setDescription(const std::string& d)
{
  if (d.size() > 4000)
  {
    LOG_WARN("Truncating description to 4000 characters");
    this->_description = d.substr(0, 4000);
  }
  else
  {
    this->_description = d;
  }
}

TiXmlElement*
TrackSource
::toXML(const std::string& name) const
{
  TiXmlElement* e = new TiXmlElement(name);
  e->SetAttribute("xsi:type", "TrackSource");
  e->LinkEndChild(XMLIOBase::toXML("trackerName", this->_name));
  e->LinkEndChild(XMLIOBase::toXML("trackerDescription", this->_description));
  return e;
}

TrackSource
::TrackSource(const TiXmlElement* elem)
{
  std::string description;
  XMLIOBase::fromXML(elem, "trackerName", this->_name);
  XMLIOBase::fromXML(elem, "trackerDescription", description);

  // Not set directly in order to reuse length checking
  this->setDescription(description);
}

STANAG_4676_FROMXML(TrackSource)

} // End STANAG_4676 namespace

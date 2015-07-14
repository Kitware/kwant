/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "DataSource.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::DataSource);

namespace STANAG_4676
{

/// \brief Initialize the DataSource
/// \param name Provides the name of the data source.
/// \param t Provides the time of the data source.
DataSource
::DataSource(const std::string& name, const boost::posix_time::ptime& t)
  : _name(name), _time(t)
{
}

/// \brief Provides the name of the data source.  This would be the image file
///        name for WAMI or the frame number for FMV.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  DataSource,
  std::string,
  Name,
  _name)

/// \brief Provides the time of the data source.  This would be the time the
///        image was collected for WAMI or the GPS time of the FMV frame.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  DataSource,
  boost::posix_time::ptime,
  Time,
  _time)

TiXmlElement*
DataSource
::toXML(const std::string& name) const
{
  TiXmlElement* e = new TiXmlElement(name);
  e->SetAttribute("xsi:type", "DataSource");
  e->LinkEndChild(XMLIOBase::toXML("sourceName", this->_name));
  e->LinkEndChild(XMLIOBase::toXML("sourceTime", this->_time));
  return e;
}

DataSource
::DataSource(const TiXmlElement* elem)
{
  XMLIOBase::fromXML(elem, "sourceName", this->_name);
  XMLIOBase::fromXML(elem, "sourceTime", this->_time);
}

STANAG_4676_FROMXML(DataSource)

} // End STANAG_4676 namespace

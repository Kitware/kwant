/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "Velocity.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::Velocity);

#include "LocalCartesianVelocity.h"

namespace STANAG_4676
{

Velocity::Velocity(void)
{
}

TiXmlElement*
Velocity
::toXML(const std::string& name) const
{
  TiXmlElement* e = new TiXmlElement(name);
  e->SetAttribute("xsi:type", "Velocity");
  return e;
}

Velocity::sptr
Velocity
::fromXML(const TiXmlElement* elem, int* error_count)
{
  const std::string* const type = elem->Attribute(std::string("xsi:type"));
  if (type)
  {
    STANAG_4676_FROMXML_DOWNCAST(elem, type, LocalCartesianVelocity)

    XMLIOBase::typeError(error_count, elem, *type);
    return sptr();
  }
  XMLIOBase::typeError(error_count, elem);
  return sptr();
}

Velocity
::Velocity(const TiXmlElement* /*elem*/)
{
}

} // End STANAG_4676 namespace

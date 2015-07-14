/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "PixelPosition.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::PixelPosition);

namespace STANAG_4676
{

/// \brief Initialize
PixelPosition
::PixelPosition(int x, int y)
  : _x(x), _y(y)
{
}

/// \brief Provides the pixel x location of the track point in an image with
///        the upper left corner being 0, 0.
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  PixelPosition,
  int,
  X,
  _x)

/// \brief Provides the pixel y location of the track point in an image with
///        the upper left corner being 0, 0.
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  PixelPosition,
  int,
  Y,
  _y)

TiXmlElement*
PixelPosition
::toXML(const std::string& name) const
{
  TiXmlElement* e = Position::toXML(name);
  e->SetAttribute("xsi:type", "PixelPosition");
  e->LinkEndChild(XMLIOBase::toXML("x", this->_x));
  e->LinkEndChild(XMLIOBase::toXML("y", this->_y));
  return e;
}

PixelPosition
::PixelPosition(const TiXmlElement* elem)
  : Position(elem)
{
  XMLIOBase::fromXML(elem, "x", this->_x);
  XMLIOBase::fromXML(elem, "y", this->_y);
}

STANAG_4676_FROMXML(PixelPosition)

} // End STANAG_4676 namespace

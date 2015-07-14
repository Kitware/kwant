/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_PIXELPOSITION_H_
#define VIDTK_LIBRARY_STANAG_4676_PIXELPOSITION_H_

#include <stanag_4676/Position.h>

namespace STANAG_4676
{

/// \brief Provides the pixel location of the track point in an image in x and
///        y.
class PixelPosition : public Position
{
public:
  typedef vbl_smart_ptr<PixelPosition> sptr;

  PixelPosition(int x, int y);

  virtual ~PixelPosition(void) { }

  STANAG_4676_IMPLEMENT_DOWNCAST(PixelPosition)

  STANAG_4676_DECLARE_POD_ATTRIBUTE(int, X)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(int, Y)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  PixelPosition(const TiXmlElement* elem);

private:
  int _x;
  int _y;
};

} // End STANAG_4676 namespace

#endif

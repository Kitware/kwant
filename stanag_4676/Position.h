/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_POSITION_H_
#define VIDTK_LIBRARY_STANAG_4676_POSITION_H_

#include <stanag_4676/XMLIOBase.h>

namespace STANAG_4676
{

class GeodeticPosition;
class LocalCartesianPosition;
class PixelPosition;

/// \brief Provides an estimate of the position of an object or feature.
class Position : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<Position> sptr;

  virtual ~Position(void) = 0;

  STANAG_4676_DECLARE_DOWNCAST(GeodeticPosition)
  STANAG_4676_DECLARE_DOWNCAST(LocalCartesianPosition)
  STANAG_4676_DECLARE_DOWNCAST(PixelPosition)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  Position(void);
  Position(const TiXmlElement* elem);
};
inline Position::~Position(void) { }

} // End STANAG_4676 namespace

#endif

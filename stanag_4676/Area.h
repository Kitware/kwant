/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_AREA_H_
#define VIDTK_LIBRARY_STANAG_4676_AREA_H_

#include <stanag_4676/XMLIOBase.h>

namespace STANAG_4676
{

class CircularArea;
class PolygonArea;

/// \brief Provides parameters of a bounded region on a plane or surface.
class Area : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<Area> sptr;

  virtual ~Area(void) = 0;

  STANAG_4676_DECLARE_DOWNCAST(CircularArea)
  STANAG_4676_DECLARE_DOWNCAST(PolygonArea)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  Area(void);
  Area(const TiXmlElement* elem);
};
inline Area::~Area(void) { }

} // End STANAG_4676 namespace

#endif

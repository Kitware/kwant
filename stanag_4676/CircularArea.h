/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_CIRCULARAREA_H_
#define VIDTK_LIBRARY_STANAG_4676_CIRCULARAREA_H_

#include <stanag_4676/Area.h>
#include <stanag_4676/Position.h>

namespace STANAG_4676
{

/// \brief Provides parameters of a bounded region defined by its center point
///        and radius.
class CircularArea : public Area
{
public:
  typedef vbl_smart_ptr<CircularArea> sptr;

  CircularArea(Position::sptr center, double radius);

  STANAG_4676_IMPLEMENT_DOWNCAST(CircularArea)

  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(Position::sptr, Center)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, Radius)

  virtual ~CircularArea(void) { };

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  CircularArea(const TiXmlElement* elem);

private:
  Position::sptr _center;
  double _radius;
};

} // End STANAG_4676 namespace

#endif

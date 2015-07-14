/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_POLYGONAREA_H_
#define VIDTK_LIBRARY_STANAG_4676_POLYGONAREA_H_

#include <stanag_4676/Area.h>
#include <stanag_4676/Position.h>

namespace STANAG_4676
{

/// \brief Provides the parameters of a bounded region defined by a series of 3
///        or more vertices or boundary points.
class PolygonArea : public Area
{
public:
  typedef vbl_smart_ptr<PolygonArea> sptr;

  PolygonArea(void);

  virtual ~PolygonArea(void) { }

  STANAG_4676_IMPLEMENT_DOWNCAST(PolygonArea)

  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::vector<Position::sptr>, Points)

  void addPoint(const Position::sptr& p);

  TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  PolygonArea(const TiXmlElement* elem);

private:
  std::vector<Position::sptr> _boundaryPoints;
};

} // End STANAG_4676 namespace

#endif

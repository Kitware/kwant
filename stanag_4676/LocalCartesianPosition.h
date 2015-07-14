/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_LOCALCARTESIANPOSITION_H_
#define VIDTK_LIBRARY_STANAG_4676_LOCALCARTESIANPOSITION_H_

#include <stanag_4676/Position.h>
#include <stanag_4676/LocalCoordinateSystem.h>

namespace STANAG_4676
{

/// \brief Provides an estimate of the position of an object, expressed in a
///        local Cartesian reference frame (LocalCoordinateSystem).
class LocalCartesianPosition : public Position
{
public:
  typedef vbl_smart_ptr<LocalCartesianPosition> sptr;

  LocalCartesianPosition(double posx, double posy, double posz,
                         const LocalCoordinateSystem::sptr& localSystem);

  virtual ~LocalCartesianPosition(void) { }

  STANAG_4676_IMPLEMENT_DOWNCAST(LocalCartesianPosition)

  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, PosX)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, PosY)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, PosZ)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(LocalCoordinateSystem::sptr, LocalSystem)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  LocalCartesianPosition(const TiXmlElement* elem);

private:
  double _posx;
  double _posy;
  double _posz;
  LocalCoordinateSystem::sptr _localSystem;
};

} // End STANAG_4676 namespace

#endif

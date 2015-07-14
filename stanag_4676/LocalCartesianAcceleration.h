/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_LOCALCARTESIANACCELERATION_H_
#define VIDTK_LIBRARY_STANAG_4676_LOCALCARTESIANACCELERATION_H_

#include <stanag_4676/Acceleration.h>

namespace STANAG_4676
{

/// \brief Provides an estimate of the three-dimensional components of
///        acceleration, expressed in a Cartesian coordinate system.
class LocalCartesianAcceleration : public Acceleration
{
public:
  typedef vbl_smart_ptr<LocalCartesianAcceleration> sptr;

  LocalCartesianAcceleration(double accx, double accy, double accz);

  virtual ~LocalCartesianAcceleration(void) { }

  STANAG_4676_IMPLEMENT_DOWNCAST(LocalCartesianAcceleration)

  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, AccX)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, AccY)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, AccZ)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  LocalCartesianAcceleration(const TiXmlElement* elem);

private:
  double _accx;
  double _accy;
  double _accz;
};

} // End STANAG_4676 namespace

#endif

/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_LOCALCARTESIANVELOCITY_H_
#define VIDTK_LIBRARY_STANAG_4676_LOCALCARTESIANVELOCITY_H_

#include <stanag_4676/Velocity.h>

namespace STANAG_4676
{

/// \brief Provides the matrix of covariances related to the estimated state
///        vector.
class LocalCartesianVelocity : public Velocity
{
public:
  typedef vbl_smart_ptr<LocalCartesianVelocity> sptr;

  LocalCartesianVelocity(double velx, double vely, double velz);

  virtual ~LocalCartesianVelocity(void) { }

  STANAG_4676_IMPLEMENT_DOWNCAST(LocalCartesianVelocity)

  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, VelX)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, VelY)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, VelZ)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  LocalCartesianVelocity(const TiXmlElement* elem);

private:
  double _velx;
  double _vely;
  double _velz;
};

} // End STANAG_4676 namespace

#endif

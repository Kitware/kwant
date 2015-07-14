/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_VELOCITY_H_
#define VIDTK_LIBRARY_STANAG_4676_VELOCITY_H_

#include <stanag_4676/XMLIOBase.h>

namespace STANAG_4676
{

class LocalCartesianVelocity;

/// \brief Provides the three-dimensional components of velocity.
///
/// In a Cartesian Coordinate System the velocity components are v<sub>x</sub>,
/// v<sub>y</sub> and v<sub>z</sub>, where velocity is a vector measurement of
/// the rate and direction of motion or, in other terms, the rate and direction
/// of the change in the position of an object. The scalar (absolute value)
/// magnitude of the velocity vector is the speed of the motion.
///
/// In calculus terms, velocity is the first derivative of position with
/// respect to time.
/// <blockquote><pre>
/// v<sub>x</sub> = dx/dt
/// v<sub>y</sub> = dy/dt
/// v<sub>z</sub> = dz/dt
/// </pre></blockquote>
///
/// The units for velocity are m/s (meters per second).
class Velocity : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<Velocity> sptr;

  virtual ~Velocity(void) = 0;

  STANAG_4676_DECLARE_DOWNCAST(LocalCartesianVelocity)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  Velocity(void);
  Velocity(const TiXmlElement* elem);
};
inline Velocity::~Velocity(void) { }

} // End STANAG_4676 namespace

#endif

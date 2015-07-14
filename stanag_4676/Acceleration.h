/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_ACCELERATION_H_
#define VIDTK_LIBRARY_STANAG_4676_ACCELERATION_H_

#include <stanag_4676/XMLIOBase.h>

namespace STANAG_4676
{

class LocalCartesianAcceleration;

/// \brief Provides the three-dimensional components of acceleration.
///
/// Acceleration is the rate of change of velocity as a function of time. It is
/// a vector.  In a Cartesian Coordinate System the velocity components are
/// a<sub>x</sub>, a<sub>y</sub> and a<sub>z</sub>.
///
/// In calculus terms, acceleration is the second
/// derivative of position with respect to time.
/// <blockquote><pre>
/// a<sub>x</sub> = d<sup>2</sup>x/dt<sup>2</sup>
/// a<sub>y</sub> = d<sup>2</sup>y/dt<sup>2</sup>
/// a<sub>z</sub> = d<sup>2</sup>z/dt<sup>2</sup>
/// </pre></blockquote>
///
/// Or, alternately, acceleration is the first derivative of the velocity with
/// respect to time.
/// <blockquote><pre>
/// a<sub>x</sub> = dv<sub>x</sub>/dt
/// a<sub>y</sub> = dv<sub>y</sub>/dt
/// a<sub>z</sub> = dv<sub>z</sub>/dt
/// </pre></blockquote>
///
/// The units for acceleration are m/s<sup>2</sup> (meters per second squared,
/// or meters per second per second).
class Acceleration : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<Acceleration> sptr;

  virtual ~Acceleration(void) = 0;

  STANAG_4676_DECLARE_DOWNCAST(LocalCartesianAcceleration)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  Acceleration(void);
  Acceleration(const TiXmlElement* elem);
};
inline Acceleration::~Acceleration(void) { }

} // End STANAG_4676 namespace

#endif

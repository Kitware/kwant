/*ckwg +5
 * Copyright 2011-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_DESCRIPTOR_MOTION_H
#define INCL_DESCRIPTOR_MOTION_H

#include <iostream>

typedef struct descriptor_motion_type
{
  double ground_pos_x;
  double ground_pos_y;
  double ground_speed;
  double ground_acceleration;
  double heading;
  double delta_heading;
  double exp_heading;
  double ang_momentum;
  double curvature;

  descriptor_motion_type(void)
    : ground_pos_x(0.0),
      ground_pos_y(0.0),
      ground_speed(0.0),
      ground_acceleration(0.0),
      heading(0.0),
      delta_heading(0.0),
      exp_heading(0.0),
      ang_momentum(0.0),
      curvature(0.0)
  { }

  bool operator==(const descriptor_motion_type& a) const
  {
    return (this->ground_pos_x == a.ground_pos_x) &&
      (this->ground_pos_y == a.ground_pos_y) &&
      (this->ground_speed == a.ground_speed) &&
      (this->ground_acceleration == a.ground_acceleration) &&
      (this->heading == a.heading) &&
      (this->delta_heading == a.delta_heading) &&
      (this->exp_heading == a.exp_heading) &&
      (this->ang_momentum == a.ang_momentum) &&
      (this->curvature == a.curvature);
  }

} descriptor_motion_type;

std::ostream& operator<<( std::ostream& os, const descriptor_motion_type& );
std::istream& operator>>( std::istream& is, descriptor_motion_type& );

#endif /* INCL_DESCRIPTOR_MOTION_H */

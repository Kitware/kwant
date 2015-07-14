/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "TrackPointDetail.h"
#include <vbl/vbl_smart_ptr.txx>
#include <limits>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::TrackPointDetail);

namespace STANAG_4676
{

/// \brief Initialize
TrackPointDetail
::TrackPointDetail(Position::sptr position, CovarianceMatrix::sptr cov)
  : _position(position), _velocity(NULL), _acceleration(NULL),
    _covarianceMatrix(cov)
{
}

/// \brief Provides the position of an object being tracked.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackPointDetail,
  Position::sptr,
  Position,
  _position)

/// \brief Provides the velocity of an object being tracked.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackPointDetail,
  Velocity::sptr,
  Velocity,
  _velocity)

/// \brief Provides the acceleration of an object being tracked.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackPointDetail,
  Acceleration::sptr,
  Acceleration,
  _acceleration)

/// \brief Provides the covariance matrix related to the state vector
///        associated with a reported track point.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  TrackPointDetail,
  CovarianceMatrix::sptr,
  CovarianceMatrix,
  _covarianceMatrix)

TiXmlElement*
TrackPointDetail
::toXML(const std::string& name) const
{
  TiXmlElement* e = new TiXmlElement(name);
  e->SetAttribute("xsi:type", "TrackPointDetail");
  e->LinkEndChild(this->_position->toXML("pointDetailPosition"));
  if (this->_velocity)
  {
    e->LinkEndChild(this->_velocity->toXML("pointDetailVelocity"));
  }
  if (this->_acceleration)
  {
    e->LinkEndChild(this->_acceleration->toXML("pointDetailAcceleration"));
  }
  e->LinkEndChild(this->_covarianceMatrix->toXML("pointDetailCovarianceMatrix"));
  return e;
}

TrackPointDetail
::TrackPointDetail(const TiXmlElement* elem)
{
  XMLIOBase::fromXML<Position>(elem, "pointDetailPosition", this->_position);
  XMLIOBase::fromXML<Velocity>(elem, "pointDetailVelocity",
                               this->_velocity, false);
  XMLIOBase::fromXML<Acceleration>(elem, "pointDetailAcceleration",
                                   this->_acceleration, false);
  XMLIOBase::fromXML<CovarianceMatrix>(elem, "pointDetailCovarianceMatrix",
                                       this->_covarianceMatrix);
}

STANAG_4676_FROMXML(TrackPointDetail)

} // End STANAG_4676 namespace

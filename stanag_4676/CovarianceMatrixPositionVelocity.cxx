/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "CovarianceMatrixPositionVelocity.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::CovarianceMatrixPositionVelocity);

namespace STANAG_4676
{

/// \brief Initialize
CovarianceMatrixPositionVelocity
::CovarianceMatrixPositionVelocity(
  CovarianceMatrixPosition::sptr pos,
  double covVelxVelx, double covVelyVely, double covVelzVelz,
  double covVelxVely, double covVelxVelz, double covVelyVelz)
  : CovarianceMatrixPosition(pos),
    _covVelxVelx(covVelxVelx), _covVelyVely(covVelyVely),
    _covVelzVelz(covVelzVelz), _covVelxVely(covVelxVely),
    _covVelxVelz(covVelxVelz), _covVelyVelz(covVelyVelz)
{
}

/// \brief Provides an estimate of the variance in the x component of velocity
///        at the time of the report, expressed in meters squared per seconds
///        squared (m<sup>2</sup>/s<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CovarianceMatrixPositionVelocity,
  double,
  CovVelxVelx,
  _covVelxVelx)

/// \brief Provides an estimate of the variance in the y component of velocity
///        at the time of the report, expressed in meters squared per seconds
///        squared (m<sup>2</sup>/s<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CovarianceMatrixPositionVelocity,
  double,
  CovVelyVely,
  _covVelyVely)

/// \brief Provides an estimate of the variance in the z component of velocity
///        at the time of the report, expressed in meters squared per seconds
///        squared (m<sup>2</sup>/s<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CovarianceMatrixPositionVelocity,
  double,
  CovVelzVelz,
  _covVelzVelz)

/// \brief Provides an estimate of the covariance between the x and y
///        components of velocity, expressed in meters squared per seconds
///        squared (m<sup>2</sup>/s<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CovarianceMatrixPositionVelocity,
  double,
  CovVelxVely,
  _covVelxVely)

/// \brief Provides an estimate of the covariance between the x and z
///        components of velocity, expressed in meters squared per seconds
///        squared (m<sup>2</sup>/s<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CovarianceMatrixPositionVelocity,
  double,
  CovVelxVelz,
  _covVelxVelz)

/// \brief Provides an estimate of the covariance between the y and z
///        components of velocity, expressed in meters squared per seconds
///        squared (m<sup>2</sup>/s<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CovarianceMatrixPositionVelocity,
  double,
  CovVelyVelz,
  _covVelyVelz)

TiXmlElement*
CovarianceMatrixPositionVelocity
::toXML(const std::string& name) const
{
  TiXmlElement* e = CovarianceMatrixPosition::toXML(name);
  e->SetAttribute("xsi:type", "CovarianceMatrixPositionVelocity");
  e->LinkEndChild(XMLIOBase::toXML("covVelxVelx", this->_covVelxVelx));
  e->LinkEndChild(XMLIOBase::toXML("covVelyVely", this->_covVelyVely));
  if (this->_covVelzVelz != std::numeric_limits<double>::min())
  {
    e->LinkEndChild(XMLIOBase::toXML("covVelzVelz", this->_covVelzVelz));
  }
  if (this->_covVelxVely != std::numeric_limits<double>::min())
  {
    e->LinkEndChild(XMLIOBase::toXML("covVelxVely", this->_covVelxVely));
  }
  if (this->_covVelxVelz != std::numeric_limits<double>::min())
  {
    e->LinkEndChild(XMLIOBase::toXML("covVelxVelz", this->_covVelxVelz));
  }
  if (this->_covVelyVelz != std::numeric_limits<double>::min())
  {
    e->LinkEndChild(XMLIOBase::toXML("covVelyVelz", this->_covVelyVelz));
  }
  return e;
}

CovarianceMatrixPositionVelocity
::CovarianceMatrixPositionVelocity(const TiXmlElement* elem)
  : CovarianceMatrixPosition(elem)
{
  XMLIOBase::fromXML(elem, "covVelxVelx", this->_covVelxVelx);
  XMLIOBase::fromXML(elem, "covVelyVely", this->_covVelyVely);
  XMLIOBase::fromXML(elem, "covVelzVelz", this->_covVelzVelz, false);
  XMLIOBase::fromXML(elem, "covVelxVely", this->_covVelxVely, false);
  XMLIOBase::fromXML(elem, "covVelxVelz", this->_covVelxVelz, false);
  XMLIOBase::fromXML(elem, "covVelyVelz", this->_covVelyVelz, false);
}

STANAG_4676_FROMXML(CovarianceMatrixPositionVelocity)

} // End STANAG_4676 namespace

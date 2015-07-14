/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "CovarianceMatrixPosition.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::CovarianceMatrixPosition);

namespace STANAG_4676
{

/// \brief Initialize
CovarianceMatrixPosition
::CovarianceMatrixPosition(
  double covPosxPosx, double covPosyPosy, double covPoszPosz,
  double covPosxPosy, double covPosxPosz, double covPosyPosz)
  : _covPosxPosx(covPosxPosx), _covPosyPosy(covPosyPosy),
    _covPoszPosz(covPoszPosz), _covPosxPosy(covPosxPosy),
    _covPosxPosz(covPosxPosz), _covPosyPosz(covPosyPosz)
{
}

CovarianceMatrixPosition
::CovarianceMatrixPosition(CovarianceMatrixPosition::sptr pos)
  : _covPosxPosx(pos->_covPosxPosx), _covPosyPosy(pos->_covPosyPosy),
    _covPoszPosz(pos->_covPoszPosz), _covPosxPosy(pos->_covPosxPosy),
    _covPosxPosz(pos->_covPosxPosz), _covPosyPosz(pos->_covPosyPosz)
{
}

/// \brief Provides an estimate of the variance in the x component of
///        position at the time of the report, expressed in meters squared
///        (m<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CovarianceMatrixPosition,
  double,
  CovPosxPosx,
  _covPosxPosx)

/// \brief Provides an estimate of the variance in the y component of
///        position at the time of the report, expressed in meters squared
///        (m<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CovarianceMatrixPosition,
  double,
  CovPosyPosy,
  _covPosyPosy)

/// \brief Provides an estimate of the variance in the z component of
///        position at the time of the report, expressed in meters squared
///        (m<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CovarianceMatrixPosition,
  double,
  CovPoszPosz,
  _covPoszPosz)

/// \brief Provides an estimate of the covariance between the x and y
///        components of position, expressed in meters squared (m<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CovarianceMatrixPosition,
  double,
  CovPosxPosy,
  _covPosxPosy)

/// \brief Provides an estimate of the covariance between the x and z
///        components of position, expressed in meters squared (m<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CovarianceMatrixPosition,
  double,
  CovPosxPosz,
  _covPosxPosz)

/// \brief Provides an estimate of the covariance between the y and z
///        components of position, expressed in meters squared (m<sup>2</sup>).
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  CovarianceMatrixPosition,
  double,
  CovPosyPosz,
  _covPosyPosz)

TiXmlElement*
CovarianceMatrixPosition
::toXML(const std::string& name) const
{
  TiXmlElement* e = CovarianceMatrix::toXML(name);
  e->SetAttribute("xsi:type", "CovarianceMatrixPosition");
  e->LinkEndChild(XMLIOBase::toXML("covPosxPosx", this->_covPosxPosx));
  e->LinkEndChild(XMLIOBase::toXML("covPosyPosy", this->_covPosyPosy));
  if (this->_covPoszPosz != std::numeric_limits<double>::min())
  {
    e->LinkEndChild(XMLIOBase::toXML("covPoszPosz", this->_covPoszPosz));
  }
  if (this->_covPosxPosy != std::numeric_limits<double>::min())
  {
    e->LinkEndChild(XMLIOBase::toXML("covPosxPosy", this->_covPosxPosy));
  }
  if (this->_covPosxPosz != std::numeric_limits<double>::min())
  {
    e->LinkEndChild(XMLIOBase::toXML("covPosxPosz", this->_covPosxPosz));
  }
  if (this->_covPosyPosz != std::numeric_limits<double>::min())
  {
    e->LinkEndChild(XMLIOBase::toXML("covPosyPosz", this->_covPosyPosz));
  }
  return e;
}

CovarianceMatrixPosition
::CovarianceMatrixPosition(const TiXmlElement* elem)
  : CovarianceMatrix(elem)
{
  XMLIOBase::fromXML(elem, "covPosxPosx", this->_covPosxPosx);
  XMLIOBase::fromXML(elem, "covPosyPosy", this->_covPosyPosy);
  XMLIOBase::fromXML(elem, "covPoszPosz", this->_covPoszPosz, false);
  XMLIOBase::fromXML(elem, "covPosxPosy", this->_covPosxPosy, false);
  XMLIOBase::fromXML(elem, "covPosxPosz", this->_covPosxPosz, false);
  XMLIOBase::fromXML(elem, "covPosyPosz", this->_covPosyPosz, false);
}

STANAG_4676_FROMXML(CovarianceMatrixPosition)

} // End STANAG_4676 namespace

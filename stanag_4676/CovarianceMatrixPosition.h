/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_COVARIANCEMATRIXPOSITION_H_
#define VIDTK_LIBRARY_STANAG_4676_COVARIANCEMATRIXPOSITION_H_

#include <limits>
#include <stanag_4676/CovarianceMatrix.h>

namespace STANAG_4676
{

/// \brief Provides the matrix of covariances related to the estimated position
///        vector.
class CovarianceMatrixPosition : public CovarianceMatrix
{
public:
  typedef vbl_smart_ptr<CovarianceMatrixPosition> sptr;

  CovarianceMatrixPosition(
    double covPosxPosx, double covPosyPosy,
    double covPoszPosz = std::numeric_limits<double>::min(),
    double covPosxPosy = std::numeric_limits<double>::min(),
    double covPosxPosz = std::numeric_limits<double>::min(),
    double covPosyPosz = std::numeric_limits<double>::min());

  CovarianceMatrixPosition(CovarianceMatrixPosition::sptr pos);

  STANAG_4676_IMPLEMENT_DOWNCAST(CovarianceMatrixPosition)

  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, CovPosxPosx)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, CovPosyPosy)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, CovPoszPosz)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, CovPosxPosy)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, CovPosxPosz)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, CovPosyPosz)

  virtual ~CovarianceMatrixPosition(void) { };

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  CovarianceMatrixPosition(const TiXmlElement* elem);

protected:
  double _covPosxPosx;
  double _covPosyPosy;
  double _covPoszPosz;
  double _covPosxPosy;
  double _covPosxPosz;
  double _covPosyPosz;
};

} // End STANAG_4676 namespace

#endif

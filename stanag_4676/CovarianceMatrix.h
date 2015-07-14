/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_COVARIANCEMATRIX_H_
#define VIDTK_LIBRARY_STANAG_4676_COVARIANCEMATRIX_H_

#include <stanag_4676/XMLIOBase.h>

namespace STANAG_4676
{

class CovarianceMatrixPosition;
class CovarianceMatrixPositionVelocity;

/// \brief Provides the matrix of covariances related to the estimated state
///        vector.
class CovarianceMatrix : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<CovarianceMatrix> sptr;

  virtual ~CovarianceMatrix(void) = 0;

  STANAG_4676_DECLARE_DOWNCAST(CovarianceMatrixPosition)
  STANAG_4676_DECLARE_DOWNCAST(CovarianceMatrixPositionVelocity)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  CovarianceMatrix(void);
  CovarianceMatrix(const TiXmlElement* elem);
};
inline CovarianceMatrix::~CovarianceMatrix(void) { }

} // End STANAG_4676 namespace

#endif

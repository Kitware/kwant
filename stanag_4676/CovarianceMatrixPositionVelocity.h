/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_COVARIANCEMATRIXPOSITIONVELOCITY_H_
#define VIDTK_LIBRARY_STANAG_4676_COVARIANCEMATRIXPOSITIONVELOCITY_H_

#include <stanag_4676/CovarianceMatrixPosition.h>

namespace STANAG_4676
{

/// \brief Provides the matrix of covariances related to the estimated position
//         and velocity vectors.
class CovarianceMatrixPositionVelocity : public CovarianceMatrixPosition
{
public:
  typedef vbl_smart_ptr<CovarianceMatrixPositionVelocity> sptr;

  CovarianceMatrixPositionVelocity(
    CovarianceMatrixPosition::sptr pos,
    double covVelxVelx, double covVelyVely,
    double covVelzVelz = std::numeric_limits<double>::min(),
    double covVelxVely = std::numeric_limits<double>::min(),
    double covVelxVelz = std::numeric_limits<double>::min(),
    double covVelyVelz = std::numeric_limits<double>::min());

  STANAG_4676_IMPLEMENT_DOWNCAST(CovarianceMatrixPositionVelocity)

  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, CovVelxVelx)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, CovVelyVely)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, CovVelzVelz)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, CovVelxVely)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, CovVelxVelz)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(double, CovVelyVelz)

  virtual ~CovarianceMatrixPositionVelocity(void) { };

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  CovarianceMatrixPositionVelocity(const TiXmlElement* elem);

  double _covVelxVelx;
  double _covVelyVely;
  double _covVelzVelz;
  double _covVelxVely;
  double _covVelxVelz;
  double _covVelyVelz;
};

} // End STANAG_4676 namespace

#endif

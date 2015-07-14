/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_TRACKPOINTDETAIL_H_
#define VIDTK_LIBRARY_STANAG_4676_TRACKPOINTDETAIL_H_

#include <stanag_4676/XMLIOBase.h>
#include <stanag_4676/Position.h>
#include <stanag_4676/Velocity.h>
#include <stanag_4676/Acceleration.h>
#include <stanag_4676/CovarianceMatrix.h>

namespace STANAG_4676
{

/// \brief Provides detailed information related to a track point.
class TrackPointDetail : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<TrackPointDetail> sptr;

  TrackPointDetail(Position::sptr position, CovarianceMatrix::sptr cov);

  virtual ~TrackPointDetail(void) { }

  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(Position::sptr, Position)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(Velocity::sptr, Velocity)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(Acceleration::sptr, Acceleration)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(CovarianceMatrix::sptr, CovarianceMatrix)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
    TrackPointDetail(const TiXmlElement* elem);

private:
  Position::sptr _position;
  Velocity::sptr _velocity;
  Acceleration::sptr _acceleration;
  CovarianceMatrix::sptr _covarianceMatrix;
};

} // End STANAG_4676 namespace

#endif

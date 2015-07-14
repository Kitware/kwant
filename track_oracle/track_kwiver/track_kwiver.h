/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_KWIVER_H
#define INCL_TRACK_KWIVER_H

#include <track_oracle/track_base.h>


namespace vidtk
{

struct track_kwiver_type: public track_base< track_kwiver_type >
{
  // The KWIVER format is special; it has no predefined types.
  // However, the file format manager requires an instance of
  // some representation of the schema.
};

} // vidtk

#endif

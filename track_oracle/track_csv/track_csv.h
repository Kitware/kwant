/*ckwg +5
 * Copyright 2011-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_CSV_H
#define INCL_TRACK_CSV_H

#include <track_oracle/track_base.h>

namespace vidtk
{


struct track_csv_type: public track_base< track_csv_type >
{
  // The CSV format is special; it has no predefined types.
  // However, the file format manager requires an instance
  // of some representation of the schema.
};

} // \namespace vidtk

#endif

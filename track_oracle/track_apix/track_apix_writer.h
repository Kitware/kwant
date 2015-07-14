/*ckwg +5
 * Copyright 2010 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_WRITER_APIX_H
#define INCL_TRACK_WRITER_APIX_H

#include <string>
#include <track_oracle/track_oracle.h>

namespace vidtk
{

struct track_apix_writer
{

// only one track per shapefile

static
bool
write( track_handle_type trk, const std::string& filename, const std::string time_format_str = "%d-%02d-%02d T %02d:%02d:%02d.%03dZ" );

};

} // namespace

#endif

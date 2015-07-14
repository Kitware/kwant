/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_SCHEMA_FACTORY_H
#define INCL_SCHEMA_FACTORY_H

//
// Construct a schema, currently by cloning fields out of known
// file formats (hence the coupling with file_format_manager rather
// than with the track_oracle core.)
//

#include <string>

namespace vidtk
{

class track_base_impl;

namespace schema_factory
{

bool clone_field_into_schema( track_base_impl& schema,
                              const std::string& name );

} // schema_factory

} // vidtk

#endif

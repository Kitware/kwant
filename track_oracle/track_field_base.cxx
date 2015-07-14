/*ckwg +5
 * Copyright 2010-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "track_field_base.h"
#include <stdexcept>

#include <logger/logger.h>
#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __vidtk_logger_auto_track_field_base_cxx__
VIDTK_LOGGER("track_field_base_cxx");

#include <track_oracle/element_descriptor.h>


using std::ostream;
using std::runtime_error;
using std::string;

namespace vidtk
{

track_field_base
::track_field_base( const string& n )
  : name(n), host(0)
{
  // field_handle now set in track_field<T>
}

track_field_base
::track_field_base( const string& n, track_field_host* h )
  : name(n), host(h)
{
}

track_field_base
::~track_field_base()
{
}

string
track_field_base
::get_field_name() const
{
  return name;
}

field_handle_type
track_field_base
::get_field_handle() const
{
  return field_handle;
}

ostream&
track_field_base
::print( ostream& os )
{
  os << "print called on asbtract field base...";
  return os;
}

//
// can't make exists() pure virtual because typeless instances are
// used as helper classes for e.g. the __parent_track field.
//

bool
track_field_base
::exists() const
{
  LOG_ERROR( "exists() called on abstract field base?");
  return false;
}

void
track_field_base
::remove_at_row( const oracle_entry_handle_type& /*row*/ )
{
  throw runtime_error("remove_row called on abstract field base");
}

void
track_field_base
::set_host( track_field_host* h )
{
  this->host = h;
}

} // namespace vidtk

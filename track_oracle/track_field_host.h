/*ckwg +5
 * Copyright 2010 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_FIELD_HOST_H_
#define INCL_TRACK_FIELD_HOST_H_

#include <track_oracle/track_oracle.h>

namespace vidtk
{

// this class exists ONLY to supply a common row
// across a row view when a track_field calls its op().

class track_field_host
{
private:
  mutable oracle_entry_handle_type cursor;

public:
  track_field_host();

  virtual ~track_field_host();

  oracle_entry_handle_type get_cursor() const;

  void set_cursor( oracle_entry_handle_type h ) const;

};

}; // namespace

#endif

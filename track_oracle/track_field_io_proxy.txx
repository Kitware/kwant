/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "track_field_io_proxy.h"

using std::ostream;

namespace vidtk
{

template< typename T >
ostream& operator<<( ostream& os, const track_field_io_proxy<T>& iop )
{
  return iop.io_ptr->to_stream( os, iop.val );
}


} // vidtk

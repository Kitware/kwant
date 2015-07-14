/*ckwg +5
 * Copyright 2013-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_DESCRIPTOR_RAW_1D_TYPE_H
#define INCL_DESCRIPTOR_RAW_1D_TYPE_H

#include <iostream>
#include <vector>

#include <vnl/vnl_vector.h>

namespace vidtk
{

struct descriptor_raw_1d_type
{
  std::vector< double > data;

  descriptor_raw_1d_type() {}
  explicit descriptor_raw_1d_type( const std::vector<double>& d ): data(d) {}
  explicit descriptor_raw_1d_type( const vnl_vector<double>& d );
  bool operator==( const descriptor_raw_1d_type& rhs ) const
  {
    if ( this->data.size() != rhs.data.size() ) return false;
    for (size_t i=0; i<this->data.size(); ++i)
    {
      if (this->data[i] != rhs.data[i]) return false;
    }
    return true;
  }
};

std::ostream& operator<<( std::ostream& os, const descriptor_raw_1d_type& d );
std::istream& operator>>( std::istream& os, descriptor_raw_1d_type& d );

} // vidtk


#endif

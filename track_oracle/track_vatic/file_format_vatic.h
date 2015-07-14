/*ckwg +5
 * Copyright 2012-2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_FILE_FORMAT_VATIC_H
#define INCL_FILE_FORMAT_VATIC_H

#include <track_oracle/file_format_base.h>
#include <track_oracle/track_vatic/track_vatic.h>

namespace vidtk
{

class file_format_vatic: public file_format_base
{
public:
  file_format_vatic(): file_format_base( TF_VATIC, "VATIC ground truth" )
  {
    this->globs.push_back( "*.txt" );
  }
  virtual ~file_format_vatic() {}

  virtual int supported_operations() const { return FF_READ; }

  // return a dynamically-allocated instance of the schema
  virtual track_base_impl* schema_instance() const { return new track_vatic_type(); }

  // Inspect the file and return true if it is of this format
  virtual bool inspect_file( const std::string& fn ) const;

  // read tracks from the file
  virtual bool read( const std::string& fn,
                     track_handle_list_type& tracks ) const;

  // read tracks from a stream
  virtual bool read( std::istream& is,
                     track_handle_list_type& tracks ) const;


};

} // vidtk

#endif

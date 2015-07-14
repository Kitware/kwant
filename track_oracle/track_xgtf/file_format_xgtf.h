/*ckwg +5
 * Copyright 2012-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_FILE_FORMAT_XGTF_H
#define INCL_FILE_FORMAT_XGTF_H

#include <track_oracle/file_format_base.h>
#include <track_oracle/track_xgtf/track_xgtf.h>

namespace vidtk
{

struct xgtf_reader_opts: public file_format_reader_opts_base
{
  bool promote_pvmoving; // if set, some activities will be converted to {p,v}moving

  xgtf_reader_opts& set_promote_pvmoving( bool p ) { this->promote_pvmoving = p; return *this; }
  xgtf_reader_opts& operator=( const file_format_reader_opts_base& rhs_base );
  virtual xgtf_reader_opts& reset() { set_promote_pvmoving( false ); return *this; }
  xgtf_reader_opts() { reset(); }
};


class file_format_xgtf: public file_format_base
{
public:
  file_format_xgtf(): file_format_base( TF_XGTF, "ViPER ground truth (using VIRAT schemas)" )
  {
    this->opts.reset();
    this->globs.push_back( "*.xgtf" );
  }
  virtual ~file_format_xgtf() {}

  virtual int supported_operations() const { return FF_READ_FILE; }

  // return a dynamically-allocated instance of the schema
  virtual track_base_impl* schema_instance() const { return new track_xgtf_type(); }

  xgtf_reader_opts& options() { return this->opts; }

  // Inspect the file and return true if it is of this format
  virtual bool inspect_file( const std::string& fn ) const;

  using file_format_base::read;

  // read tracks from the file
  virtual bool read( const std::string& fn,
                     track_handle_list_type& tracks ) const;

private:
  xgtf_reader_opts opts;

};

} // vidtk

#endif

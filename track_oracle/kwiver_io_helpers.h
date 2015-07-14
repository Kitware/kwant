/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef KWIVER_IO_HELPERS
#define KWIVER_IO_HELPERS

#include <iostream>
#include <vector>
#include <string>

#include <vgl/vgl_point_2d.h>
#include <vgl/vgl_box_2d.h>
#include <vgl/vgl_point_3d.h>
#include <utilities/timestamp.h>

namespace vidtk
{

std::ostream& kwiver_write_highprecision( std::ostream& os, double d, std::streamsize new_prec = 10 );

bool kwiver_read( const std::string& s, vgl_point_2d<double>& d );
std::ostream& kwiver_write( std::ostream& os, const vgl_point_2d<double>& d, const std::string& sep );
std::vector<std::string> kwiver_csv_header_pair( const std::string& n, const std::string& p1, const std::string& p2 );

bool kwiver_read( const std::string& s, vgl_box_2d<double>& d );
std::ostream& kwiver_write( std::ostream& os, const vgl_box_2d<double>& d, const std::string& sep );
std::vector<std::string> kwiver_box_2d_headers( const std::string& s );

bool kwiver_read( const std::string& s, vgl_point_3d<double>& d );
std::ostream& kwiver_write( std::ostream& os, const vgl_point_3d<double>& d, const std::string& sep );
std::vector<std::string> kwiver_point_3d_headers( const std::string& n );

std::pair<std::string, std::string > kwiver_ts_to_strings( const timestamp& ts );
bool kwiver_ts_string_read( const std::string& frame_str,
                            const std::string& time_str,
                            timestamp& t );
bool kwiver_read( const std::string& s, vidtk::timestamp& ts );
std::ostream& kwiver_write( std::ostream& os, const vidtk::timestamp& ts );

//
// default, unimplemented output routine for TMP
//

char kwiver_write( std::ostream& os, ... );

} // vidtk

#endif

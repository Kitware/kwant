/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_LOGGING_MAP_H
#define INCL_LOGGING_MAP_H

///
/// Reduce logging verbosity.
///

#include <string>
#include <map>

#include <logger/vidtk_logger.h>
#include <logger/location_info.h>


namespace vidtk
{
class logging_map_type
{
public:
  logging_map_type( vidtk_logger_sptr logger, const logger_ns::location_info& s  );

  logging_map_type& set_output_prefix( const std::string& s );
  bool add_msg( const std::string& msg );
  bool empty() const;
  size_t n_msgs() const;
  void dump_msgs( vidtk_logger::log_level_t level = vidtk_logger::LEVEL_INFO ) const;
  void clear();

private:
  vidtk_logger_sptr my_logger;
  logger_ns::location_info site;
  std::string output_prefix;
  std::map< std::string, std::size_t > msg_map;
};



} // vidtk



#endif

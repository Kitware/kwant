/*ckwg +5
 * Copyright 2010,2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#ifndef _VIDTK_MINI_LOGGER_FORMATTER_INT_H_
#define _VIDTK_MINI_LOGGER_FORMATTER_INT_H_

#include <logger/vidtk_logger.h>


namespace vidtk {
namespace logger_ns {

  class logger_factory;
  class vidtk_logger_mini_logger;
  class location_info;

// ----------------------------------------------------------------
/** Formatter private implementation.
 *
 *
 */
class formatter_impl
{
public:
  formatter_impl() { }
  virtual ~formatter_impl() { }


  // persistent data
  vidtk::logger_ns::logger_factory const* m_parent;
  vidtk::logger_ns::vidtk_logger_mini_logger const* m_logger;

  // per message data
  vidtk::logger_ns::location_info const* m_location;
  vidtk_logger::log_level_t m_level;
  std::string const* m_message;
  std::string const* m_realm;

}; // end class formatter_impl


}
}

#endif /* _VIDTK_MINI_LOGGER_FORMATTER_INT_H_ */

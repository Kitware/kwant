/*ckwg +5
 * Copyright 2010,2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef _VIDTK_LOGGER_LOG4CXX_H_
#define _VIDTK_LOGGER_LOG4CXX_H_

#include <logger/vidtk_logger.h>

#include <log4cxx/logger.h>

namespace vidtk {
namespace logger_ns {

class logger_factory;

// ----------------------------------------------------------------
/** Logging class.
 *
 * This class is the logger interface to the underlying logger.
 *
 * This class uses Apache log4cxx as the implementation for actual log
 * handling.
 *
 */
class vidtk_logger_log4cxx :
  public vidtk_logger
{
public:
  vidtk_logger_log4cxx( logger_factory* p, const char* const realm );
  virtual ~vidtk_logger_log4cxx();

  // Check to see if level is enabled
  virtual bool is_fatal_enabled() const;
  virtual bool is_error_enabled() const;
  virtual bool is_warn_enabled() const;
  virtual bool is_info_enabled() const;
  virtual bool is_debug_enabled() const;
  virtual bool is_trace_enabled() const;

  virtual void set_level( log_level_t lev );
  virtual log_level_t get_level() const;

  virtual void log_fatal( std::string const& msg );
  virtual void log_fatal( std::string const&                      msg,
                          vidtk::logger_ns::location_info const&  location );

  virtual void log_error( std::string const& msg );
  virtual void log_error( std::string const&                      msg,
                          vidtk::logger_ns::location_info const&  location );

  virtual void log_warn( std::string const& msg );
  virtual void log_warn( std::string const&                     msg,
                         vidtk::logger_ns::location_info const& location );

  virtual void log_info( std::string const& msg );
  virtual void log_info( std::string const&                     msg,
                         vidtk::logger_ns::location_info const& location );

  virtual void log_debug( std::string const& msg );
  virtual void log_debug( std::string const&                      msg,
                          vidtk::logger_ns::location_info const&  location );

  virtual void log_trace( std::string const& msg );
  virtual void log_trace( std::string const&                      msg,
                          vidtk::logger_ns::location_info const&  location );

  virtual void log_message( log_level_t level, std::string const& msg );
  virtual void log_message( log_level_t level, std::string const& msg,
                            vidtk::logger_ns::location_info const& location );

  // -- extended interface --
  log4cxx::LoggerPtr get_logger_impl();


protected:
  log4cxx::LoggerPtr m_loggerImpl;


private:
  void add_application_info( std::string& msg );

}; // end class


} // end namespace
} // end namespace

#endif /* _VIDTK_LOGGER_LOG4CXX_H_ */

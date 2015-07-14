/*ckwg +5
 * Copyright 2010,2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#ifndef _VIDTK_LOGGER_H_
#define _VIDTK_LOGGER_H_


#include <vbl/vbl_ref_count.h>
#include <vbl/vbl_smart_ptr.h>
#include <string>


namespace vidtk {
namespace logger_ns {

  class logger_factory;
  class location_info;
  class vidtk_mini_logger_formatter;

}

class vidtk_logger;

typedef vbl_smart_ptr < vidtk_logger > vidtk_logger_sptr;

bool set_mini_logger_formatter( vidtk_logger_sptr logger, logger_ns::vidtk_mini_logger_formatter * fmt);


// ----------------------------------------------------------------
/** Logging class.
 *
 * This class is the abstract base class for all loggers. It represents
 * the client interface to the logging facility.
 *
 * It is no accident that this looks similar to log4cxx.
 *
 */
class vidtk_logger
  : public vbl_ref_count
{
public:
  enum log_level_t {
    LEVEL_NONE = 1,
    LEVEL_TRACE,
    LEVEL_DEBUG,
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_ERROR,
    LEVEL_FATAL };

  vidtk_logger( logger_ns::logger_factory* p, const char * const realm );
  vidtk_logger( logger_ns::logger_factory* p, std::string const& realm );
  virtual ~vidtk_logger();

  // Check to see if level is enabled
  virtual bool is_fatal_enabled() const = 0;
  virtual bool is_error_enabled() const = 0;
  virtual bool is_warn_enabled()  const = 0;
  virtual bool is_info_enabled()  const = 0;
  virtual bool is_debug_enabled() const = 0;
  virtual bool is_trace_enabled() const = 0;

  virtual void set_level( log_level_t lev) = 0;
  virtual log_level_t get_level() const = 0;

  std::string get_name();

  /**
   Log a message string with the FATAL level.

   This method first checks if this logger has <code>FATAL</code>
   enabled by comparing the level of this logger with the FATAL
   level. If this logger has <code>FATAL</code> enabled, it proceeds to
   format and create a log message using the specified message.

   @param msg the message string to log.
  */
  virtual void log_fatal (std::string const & msg) = 0;

  /**
   Log a message string with the FATAL level.

   This method first checks if this logger has <code>FATAL</code>
   enabled by comparing the level of this logger with the FATAL
   level. If this logger has <code>FATAL</code> enabled, it proceeds to
   format and create a log message using the specified message and logging location.

   @param msg the message string to log.
   @param location location of source of logging request.
  */
  virtual void log_fatal (std::string const & msg,
                          vidtk::logger_ns::location_info const & location) = 0;

  /**
   Log a message string with the ERROR level.

   This method first checks if this logger has <code>ERROR</code>
   enabled by comparing the level of this logger with the ERROR
   level. If this logger has <code>ERROR</code> enabled, it proceeds to
   format and create a log message using the specified message.

   @param msg the message string to log.
  */
  virtual void log_error (std::string const & msg) = 0;

  /**
   Log a message string with the ERROR level.

   This method first checks if this logger has <code>ERROR</code>
   enabled by comparing the level of this logger with the ERROR
   level. If this logger has <code>ERROR</code> enabled, it proceeds to
   format and create a log message using the specified message and logging location.

   @param msg the message string to log.
   @param location location of source of logging request.
  */
  virtual void log_error (std::string const & msg,
                          vidtk::logger_ns::location_info const & location) = 0;

  /**
   Log a message string with the WARN level.

   This method first checks if this logger has <code>WARN</code>
   enabled by comparing the level of this logger with the WARN
   level. If this logger has <code>WARN</code> enabled, it proceeds to
   format and create a log message using the specified message.

   @param msg the message string to log.
  */
  virtual void log_warn (std::string const & msg) = 0;

  /**
   Log a message string with the WARN level.

   This method first checks if this logger has <code>WARN</code>
   enabled by comparing the level of this logger with the WARN
   level. If this logger has <code>WARN</code> enabled, it proceeds to
   format and create a log message using the specified message and logging location.

   @param msg the message string to log.
   @param location location of source of logging request.
  */
  virtual void log_warn (std::string const & msg,
                         vidtk::logger_ns::location_info const & location) = 0;

  /**
   Log a message string with the INFO level.

   This method first checks if this logger has <code>INFO</code>
   enabled by comparing the level of this logger with the INFO
   level. If this logger has <code>INFO</code> enabled, it proceeds to
   format and create a log message using the specified message.

   @param msg the message string to log.
  */
  virtual void log_info (std::string const & msg) = 0;

  /**
   Log a message string with the INFO level.

   This method first checks if this logger has <code>INFO</code>
   enabled by comparing the level of this logger with the INFO
   level. If this logger has <code>INFO</code> enabled, it proceeds to
   format and create a log message using the specified message and logging location.

   @param msg the message string to log.
   @param location location of source of logging request.
  */
  virtual void log_info (std::string const & msg,
                         vidtk::logger_ns::location_info const & location) = 0;

  /**
   Log a message string with the DEBUG level.

   This method first checks if this logger has <code>DEBUG</code>
   enabled by comparing the level of this logger with the DEBUG
   level. If this logger has <code>DEBUG</code> enabled, it proceeds to
   format and create a log message using the specified message.

   @param msg the message string to log.
  */
  virtual void log_debug (std::string const & msg) = 0;

  /**
   Log a message string with the DEBUG level.

   This method first checks if this logger has <code>DEBUG</code>
   enabled by comparing the level of this logger with the DEBUG
   level. If this logger has <code>DEBUG</code> enabled, it proceeds to
   format and create a log message using the specified message and logging location.

   @param msg the message string to log.
   @param location location of source of logging request.
  */
  virtual void log_debug (std::string const & msg,
                          vidtk::logger_ns::location_info const & location) = 0;

  /**
   Log a message string with the TRACE level.

   This method first checks if this logger has <code>TRACE</code>
   enabled by comparing the level of this logger with the TRACE
   level. If this logger has <code>TRACE</code> enabled, it proceeds to
   format and create a log message using the specified message.

   @param msg the message string to log.
  */
  virtual void log_trace (std::string const & msg) = 0;

  /**
   Log a message string with the TRACE level.

   This method first checks if this logger has <code>TRACE</code>
   enabled by comparing the level of this logger with the TRACE
   level. If this logger has <code>TRACE</code> enabled, it proceeds to
   format and create a log message using the specified message and logging location.

   @param msg the message string to log.
   @param location location of source of logging request.
  */
  virtual void log_trace (std::string const & msg,
                          vidtk::logger_ns::location_info const & location) = 0;

  /**
   Log a message string with specified level.

   This method first checks if this logger has the specified enabled
   by comparing the level of this logger with the churrent logger
   level. If this logger has this level enabled, it proceeds to
   format and create a log message using the specified message.

   @param msg the message string to log.
  */
  virtual void log_message (log_level_t level, std::string const& msg) = 0;

  /**
   Log a message string with specified level.

   This method first checks if this logger has the specified enabled
   by comparing the level of this logger with the churrent logger
   level. If this logger has this level enabled, it proceeds to format
   and create a log message using the specified message and location.

   @param msg the message string to log.
   @param location location of source of logging request.
  */
  virtual void log_message (log_level_t level, std::string const& msg,
                            vidtk::logger_ns::location_info const & location) = 0;


  /**
     Convert level code to string.

     @param lev level value to convert
  */
  char const* get_level_string(vidtk_logger::log_level_t lev) const;

protected:

  logger_ns::logger_factory* m_parent;
  std::string m_loggingRealm;

}; // end class

} // end namespace

#endif /* _VIDTK_LOGGER_H_ */

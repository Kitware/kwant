/*ckwg +5
 * Copyright 2013-2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include <logger/vidtk_logger_mini_logger.h>

#include <logger/vidtk_mini_logger_formatter.h>
#include <logger/vidtk_mini_logger_formatter_int.h>

#include <logger/logger_factory.h>
#include <logger/location_info.h>
#include <iostream>
#include <sstream>
#include <map>

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time.hpp>
#include <boost/ptr_container/ptr_map.hpp>

using namespace boost::posix_time;

namespace vidtk {
namespace logger_ns {

// -- STATIC data --
// Set up default logging stream
std::ostream* vidtk_logger_mini_logger::s_output_stream = &std::cerr;


// Use 1 mutex per stream.  This needs to be static to allow for multiple
// loggers to use the same stream and still have it locked appropriately
boost::mutex& get_stream_mtx( const std::ostream& s )
{
  static boost::shared_mutex stream_mtx_map_mtx;
  static boost::ptr_map< const std::ostream*, boost::mutex > stream_mtx_map;

  boost::shared_lock< boost::shared_mutex > stream_mtx_map_lock( stream_mtx_map_mtx );

  // create a new mutex if not already there
  if ( 0 == stream_mtx_map.count( &s ) )
  {
    boost::upgrade_lock< boost::shared_mutex > lock( stream_mtx_map_mtx );
    const std::ostream* tsp = &s;
    stream_mtx_map.insert( tsp, new boost::mutex() );
  }

  return stream_mtx_map[&s];
}


vidtk_logger_mini_logger
::vidtk_logger_mini_logger (logger_factory* p, char const* realm)
  :vidtk_logger( p, realm ),
   m_logLevel(vidtk_logger::LEVEL_TRACE),
   m_formatter(0)
{ }


vidtk_logger_mini_logger
::~vidtk_logger_mini_logger()
{
  delete m_formatter;
}


// ----------------------------------------------------------------
/* Test current log level.
 *
 *
 */
bool vidtk_logger_mini_logger
::is_fatal_enabled() const
{
  return (m_logLevel <= vidtk_logger::LEVEL_FATAL);
}


bool vidtk_logger_mini_logger
::is_error_enabled() const
{
  return (m_logLevel <= vidtk_logger::LEVEL_ERROR);
}


bool vidtk_logger_mini_logger
::is_warn_enabled() const
{
  return (m_logLevel <= vidtk_logger::LEVEL_WARN);
}


bool vidtk_logger_mini_logger
::is_info_enabled() const
{
  return (m_logLevel <= vidtk_logger::LEVEL_INFO);
}


  bool vidtk_logger_mini_logger
::is_debug_enabled() const
{
  return (m_logLevel <= vidtk_logger::LEVEL_DEBUG);
}


bool vidtk_logger_mini_logger
::is_trace_enabled() const
{
  return (m_logLevel <= vidtk_logger::LEVEL_TRACE);
}

// ----------------------------------------------------------------
/* get / set log level
 *
 *
 */
void vidtk_logger_mini_logger
::set_level( vidtk_logger::log_level_t lvl)
{
  m_logLevel = lvl;
}


vidtk_logger::log_level_t vidtk_logger_mini_logger
::get_level () const
{
  return m_logLevel;
}


// ----------------------------------------------------------------
/** Get logging stream.
 *
 * This method returns the stream used to write log messages.
 * Please note, this functions first call MUST happen at first runtime use.
 * if not, it's likely that std::cerr won't yet be initialized.
 */
std::ostream& vidtk_logger_mini_logger
::get_stream()
{
  // Make sure that any given stream only get's "imbued" once
  static std::map<std::ostream*, bool> is_imbued;
  static boost::mutex ctor_mtx;
  boost::lock_guard<boost::mutex> ctor_lock(ctor_mtx);

  if (!is_imbued[s_output_stream])
  {
    // Configure timestamp formatting
    time_facet* f = new time_facet("%Y-%m-%d %H:%M:%s");
    std::locale loc(std::locale(), f);
    {
      boost::lock_guard<boost::mutex> stream_lock(get_stream_mtx(*s_output_stream));
      s_output_stream->imbue(loc);
    }
    is_imbued[s_output_stream] = true;
  }

  return *s_output_stream;
}


// ----------------------------------------------------------------
/** Set logging stream;
 *
 *
 */
void vidtk_logger_mini_logger
::set_output_stream( std::ostream* str )
{
  s_output_stream = str;
}


// ----------------------------------------------------------------
/* Log messages
 *
 *
 */

void vidtk_logger_mini_logger
::log_fatal (std::string const & msg)
{
  if (is_fatal_enabled())
  {
    log_message (LEVEL_FATAL, msg);
  }
}


void vidtk_logger_mini_logger
::log_fatal (std::string const & msg, vidtk::logger_ns::location_info const & location)
{
  if (is_fatal_enabled())
  {
    log_message (LEVEL_FATAL, msg, location);
  }
}


void vidtk_logger_mini_logger
::log_error (std::string const & msg)
{
  if (is_error_enabled())
  {
    log_message (LEVEL_ERROR, msg);
  }
}


void vidtk_logger_mini_logger
::log_error (std::string const & msg, vidtk::logger_ns::location_info const & location)
{
  if (is_error_enabled())
  {
    log_message (LEVEL_ERROR, msg, location);
  }
}


void vidtk_logger_mini_logger
::log_warn (std::string const & msg)
{
  if (is_warn_enabled())
  {
    log_message (LEVEL_WARN, msg);
  }
}


void vidtk_logger_mini_logger
::log_warn (std::string const & msg, vidtk::logger_ns::location_info const & location)
{
  if (is_warn_enabled())
  {
    log_message (LEVEL_WARN, msg, location);
  }
}



void vidtk_logger_mini_logger
::log_info (std::string const & msg)
{
  if (is_info_enabled())
  {
    log_message (LEVEL_INFO, msg);
  }

}


void vidtk_logger_mini_logger
::log_info (std::string const & msg, vidtk::logger_ns::location_info const & location)
{
  if (is_info_enabled())
  {
    log_message (LEVEL_INFO, msg, location);
  }
}



void vidtk_logger_mini_logger
::log_debug (std::string const & msg)
{
  if (is_debug_enabled())
  {
    log_message (LEVEL_DEBUG, msg);
  }
}


void vidtk_logger_mini_logger
::log_debug (std::string const & msg, vidtk::logger_ns::location_info const & location)
{
  if (is_debug_enabled())
  {
    log_message (LEVEL_DEBUG, msg, location);
  }
}



void vidtk_logger_mini_logger
::log_trace (std::string const & msg)
{
  if (is_trace_enabled())
  {
    log_message (LEVEL_TRACE, msg);
  }
}


void vidtk_logger_mini_logger
::log_trace (std::string const & msg, vidtk::logger_ns::location_info const & location)
{
  if (is_trace_enabled())
  {
    log_message (LEVEL_TRACE, msg, location);
  }
}


// ----------------------------------------------------------------
/** Generic message writer.
 *
 *
 */
void vidtk_logger_mini_logger
::log_message ( vidtk_logger::log_level_t level, std::string const& msg)
{
  // If a formatter is specified, then use it.
  if (0 != m_formatter)
  {
    boost::lock_guard<boost::mutex> formatter_lock(m_formatter_mtx);
    log_message (level, msg, *m_formatter);
    return;
  }

  // Format this message on the stream

  // Get the current time in milliseconds, creating a formated
  // string for log message.
  ptime now = microsec_clock::local_time();

  // Ensure that multi-line messages still get the time and level prefix
  std::string level_str = get_level_string(level);
  std::string msg_part;
  std::istringstream ss(msg);

  std::ostream *s = &get_stream();
  {
    boost::lock_guard<boost::mutex> stream_lock(get_stream_mtx(*s));
    while(getline(ss, msg_part))
    {
      *s << now << ' ' << level_str << ' ' << msg_part << '\n';
    }
  }
}


// ----------------------------------------------------------------
/** Format log message.
 *
 *
 */
void vidtk_logger_mini_logger
::log_message ( vidtk_logger::log_level_t level,
                std::string const& msg,
                vidtk::logger_ns::location_info const & location)
{
  // If a formatter is specified, then use it.
  if (0 != m_formatter)
  {
    boost::lock_guard<boost::mutex> formatter_lock(m_formatter_mtx);
    log_message (level, msg, location, *m_formatter);
    return;
  }

  log_message(level, msg);
}


// ----------------------------------------------------------------
/** Register log message formatter object
 *
 *
 */
void vidtk_logger_mini_logger
::register_formatter (vidtk_mini_logger_formatter * fmt)
{
  boost::lock_guard<boost::mutex> formatter_lock(m_formatter_mtx);

  // delete any existing formatter.
  delete m_formatter;
  m_formatter = fmt;

  m_formatter->get_impl()->m_parent = m_parent;
  m_formatter->get_impl()->m_logger = this;
}


// ----------------------------------------------------------------
/** Print log message using supplied formatter.
 *
 *
 */
void vidtk_logger_mini_logger
::log_message ( vidtk_logger::log_level_t level,
                std::string const& msg,
                vidtk::logger_ns::vidtk_mini_logger_formatter & formatter)
{
  formatter.get_impl()->m_level = level;
  formatter.get_impl()->m_message = &msg;
  formatter.get_impl()->m_location = 0;
  formatter.get_impl()->m_realm = &m_loggingRealm;

  std::ostream *s = &get_stream();
  {
    boost::lock_guard<boost::mutex> stream_lock(get_stream_mtx(*s));
    formatter.format_message(*s);
  }
}


// ----------------------------------------------------------------
/** Print log message using supplied formatter.
 *
 *
 */
void vidtk_logger_mini_logger
::log_message ( vidtk_logger::log_level_t level,
                std::string const& msg,
                vidtk::logger_ns::location_info const & location,
                vidtk::logger_ns::vidtk_mini_logger_formatter & formatter)
{
  formatter.get_impl()->m_level = level;
  formatter.get_impl()->m_message = &msg;
  formatter.get_impl()->m_location = &location;
  formatter.get_impl()->m_realm = &m_loggingRealm;

  std::ostream *s = &get_stream();
  {
    boost::lock_guard<boost::mutex> stream_lock(get_stream_mtx(*s));
    formatter.format_message(*s);
  }
}


} // end namespace
} // end namespace

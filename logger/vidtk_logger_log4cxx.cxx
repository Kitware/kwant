/*ckwg +5
 * Copyright 2010,2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include <logger/vidtk_logger_log4cxx.h>

#include <logger/logger_factory.h>
#include <logger/location_info.h>
#include <sstream>

namespace vidtk {
namespace logger_ns {

vidtk_logger_log4cxx
::vidtk_logger_log4cxx( logger_factory* p, const char * const realm )
  : vidtk_logger( p, realm ),
   m_loggerImpl(0)
{
  this->m_loggerImpl = ::log4cxx::Logger::getLogger(realm);
}


vidtk_logger_log4cxx
::~vidtk_logger_log4cxx()
{ }


bool vidtk_logger_log4cxx
::is_fatal_enabled() const
{
  return this->m_loggerImpl->isFatalEnabled();
}


bool vidtk_logger_log4cxx
::is_error_enabled() const
{
  return this->m_loggerImpl->isErrorEnabled();
}


bool vidtk_logger_log4cxx
::is_warn_enabled() const
{
  return this->m_loggerImpl->isWarnEnabled();
}


bool vidtk_logger_log4cxx
::is_info_enabled() const
{
  return this->m_loggerImpl->isInfoEnabled();
}


  bool vidtk_logger_log4cxx
::is_debug_enabled() const
{
  return this->m_loggerImpl->isDebugEnabled();
}


bool vidtk_logger_log4cxx
::is_trace_enabled() const
{
  return this->m_loggerImpl->isTraceEnabled();
}

// ----------------------------------------------------------------
/* get / set log level
 *
 *
 */
void vidtk_logger_log4cxx
::set_level( vidtk_logger::log_level_t level)
{
  log4cxx::LevelPtr lvl;
  switch (level)
  {
  case LEVEL_TRACE:
    lvl = ::log4cxx::Level::getTrace();
    break;

  case LEVEL_DEBUG:
    lvl = ::log4cxx::Level::getDebug();
    break;

  case LEVEL_INFO:
    lvl = ::log4cxx::Level::getInfo();
    break;

  case LEVEL_WARN:
    lvl = ::log4cxx::Level::getWarn();
    break;

  case LEVEL_ERROR:
    lvl = ::log4cxx::Level::getError();
    break;

  case LEVEL_FATAL:
    lvl = ::log4cxx::Level::getFatal();
    break;

  default:
    // log or throw
    break;
  } // end switch

  this->m_loggerImpl->setLevel(lvl);
}


vidtk_logger::log_level_t vidtk_logger_log4cxx
::get_level() const
{
  log4cxx::LevelPtr lvl = this->m_loggerImpl->getLevel();

  if (lvl == ::log4cxx::Level::getTrace()) return LEVEL_TRACE;
  if (lvl == ::log4cxx::Level::getDebug()) return LEVEL_DEBUG;
  if (lvl == ::log4cxx::Level::getInfo())  return LEVEL_INFO;
  if (lvl == ::log4cxx::Level::getWarn())  return LEVEL_WARN;
  if (lvl == ::log4cxx::Level::getError()) return LEVEL_ERROR;
  if (lvl == ::log4cxx::Level::getFatal()) return LEVEL_FATAL;
  return LEVEL_NONE;
}

void vidtk_logger_log4cxx
::log_fatal (std::string const & msg)
{
  this->m_loggerImpl->fatal(msg);
}


void vidtk_logger_log4cxx
::log_fatal (std::string const & msg, ::vidtk::logger_ns::location_info const & location)
{
  log4cxx::spi::LocationInfo cxx_location (location.get_file_name_ptr(),
                                           location.get_method_name_ptr(),
                                           location.get_line_number());
  this->m_loggerImpl->fatal(msg, cxx_location);
}


void vidtk_logger_log4cxx
::log_error (std::string const & msg)
{
  this->m_loggerImpl->error(msg);
}


void vidtk_logger_log4cxx
::log_error (std::string const & msg, ::vidtk::logger_ns::location_info const & location)
{
  log4cxx::spi::LocationInfo cxx_location (location.get_file_name_ptr(),
                                           location.get_method_name_ptr(),
                                           location.get_line_number());
  this->m_loggerImpl->error(msg, cxx_location);
}


void vidtk_logger_log4cxx
::log_warn (std::string const & msg)
{
  this->m_loggerImpl->warn(msg);
}


void vidtk_logger_log4cxx
::log_warn (std::string const & msg, ::vidtk::logger_ns::location_info const & location)
{
  log4cxx::spi::LocationInfo cxx_location (location.get_file_name_ptr(),
                                           location.get_method_name_ptr(),
                                           location.get_line_number());
  this->m_loggerImpl->warn(msg, cxx_location);
}



void vidtk_logger_log4cxx
::log_info (std::string const & msg)
{
  this->m_loggerImpl->info(msg);
}


void vidtk_logger_log4cxx
::log_info (std::string const & msg, ::vidtk::logger_ns::location_info const & location)
{
  log4cxx::spi::LocationInfo cxx_location (location.get_file_name_ptr(),
                                           location.get_method_name_ptr(),
                                           location.get_line_number());
  this->m_loggerImpl->info(msg, cxx_location);
}



void vidtk_logger_log4cxx
::log_debug (std::string const & msg)
{
  this->m_loggerImpl->debug(msg);
}


void vidtk_logger_log4cxx
::log_debug (std::string const & msg, ::vidtk::logger_ns::location_info const & location)
{
  log4cxx::spi::LocationInfo cxx_location (location.get_file_name_ptr(),
                                           location.get_method_name_ptr(),
                                           location.get_line_number());
  this->m_loggerImpl->debug(msg, cxx_location);
}



void vidtk_logger_log4cxx
::log_trace (std::string const & msg)
{
  this->m_loggerImpl->trace(msg);
}


void vidtk_logger_log4cxx
::log_trace (std::string const & msg, ::vidtk::logger_ns::location_info const & location)
{
  log4cxx::spi::LocationInfo cxx_location (location.get_file_name_ptr(),
                                           location.get_method_name_ptr(),
                                           location.get_line_number());
  this->m_loggerImpl->trace(msg, cxx_location);
}


void vidtk_logger_log4cxx
::log_message ( vidtk_logger::log_level_t level, std::string const& msg)
{
  log4cxx::LevelPtr lvl;
  switch (level)
  {
  case LEVEL_TRACE:
    lvl = ::log4cxx::Level::getTrace();
    break;

  case LEVEL_DEBUG:
    lvl = ::log4cxx::Level::getDebug();
    break;

  case LEVEL_INFO:
    lvl = ::log4cxx::Level::getInfo();
    break;

  case LEVEL_WARN:
    lvl = ::log4cxx::Level::getWarn();
    break;

  case LEVEL_ERROR:
    lvl = ::log4cxx::Level::getError();
    break;

  case LEVEL_FATAL:
    lvl = ::log4cxx::Level::getFatal();
    break;

  default:
    break;
  } // end switch

  this->m_loggerImpl->log(lvl, msg);
}


void vidtk_logger_log4cxx
::log_message ( vidtk_logger::log_level_t level, std::string const& msg,
              ::vidtk::logger_ns::location_info const & location)
{
  log4cxx::spi::LocationInfo cxx_location (location.get_file_name_ptr(),
                                           location.get_method_name_ptr(),
                                           location.get_line_number());

  log4cxx::LevelPtr lvl;
  switch (level)
  {
  case LEVEL_TRACE:
    lvl = ::log4cxx::Level::getTrace();
    break;

  case LEVEL_DEBUG:
    lvl = ::log4cxx::Level::getDebug();
    break;

  case LEVEL_INFO:
    lvl = ::log4cxx::Level::getInfo();
    break;

  case LEVEL_WARN:
    lvl = ::log4cxx::Level::getWarn();
    break;

  case LEVEL_ERROR:
    lvl = ::log4cxx::Level::getError();
    break;

  case LEVEL_FATAL:
    lvl = ::log4cxx::Level::getFatal();
    break;

  default:
    break;
  } // end switch

  this->m_loggerImpl->log(lvl, msg, cxx_location);
}


// ----------------------------------------------------------------
/** Get logger internal implementation.
 *
 * Return pointer to the log4cxx implementaion.  This is for advanced
 * operations and is not part of the standard API.
 *
 */
::log4cxx::LoggerPtr vidtk_logger_log4cxx
::get_logger_impl()
{
  return m_loggerImpl;
}


// ----------------------------------------------------------------
/** Add application location
 *
 * This method adds the system location to the log message. Ideally
 * this information could be added by the pattern layout facility in
 * log4cxx, but that gets into modifying the base logger.
 *
 * NOTE: Currently there is an issue with inserting this text into the
 * messages. Since they are const, we would have to make a local copy
 * of the message before we can modify it, so adding this text to the
 * message is deferred.
 */
void vidtk_logger_log4cxx
::add_application_info(std::string & msg)
{
  std::stringstream loc;
  loc << "[";

  if ( ! m_parent->get_application_instance_name().empty() )
  {
    loc << m_parent->get_application_instance_name() << ":";
  }

  loc << m_parent->get_application_name();

  if ( ! m_parent->get_system_name().empty() )
  {
    loc << " @ " << m_parent->get_system_name();
  }

  loc << "] ";

  msg.insert(0, loc.str());
}


} // end namespace
} // end namespace

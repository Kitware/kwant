/*ckwg +5
 * Copyright 2010-2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include <logger/vidtk_mini_logger_formatter.h>

#include <logger/vidtk_mini_logger_formatter_int.h>
#include <logger/vidtk_logger_mini_logger.h>
#include <logger/location_info.h>
#include <logger/logger_factory.h>
#include <sstream>
#include <vpl/vpl.h>
#include <boost/date_time.hpp>

namespace vidtk {

// ----------------------------------------------------------------
/** \brief Set mini logger formatter.
 *
 * This function sets the specified formatter object as the current
 * formatter for the specified logger. Only one formatter can be
 * active. Once the formatter has been registered, the logger will
 * delete it when it is destroyed.
 *
 * @param[in] logger - logger to get new formatter.
 * @param[in] fmt - formatter object
 *
 * @return True if formatter can be set into logger; false
 * otherwise. Since the logger is be polymorphic, the usual reason
 * for not being able to set new formatter is wrong type of logger.
 */
bool set_mini_logger_formatter( vidtk_logger_sptr logger,
                                logger_ns::vidtk_mini_logger_formatter * fmt)
{
  // If we are using the mini-logger, then apply new formatter
  ::vidtk::logger_ns::vidtk_logger_mini_logger * mlp =
      dynamic_cast< ::vidtk::logger_ns::vidtk_logger_mini_logger* >(logger.ptr());
  if (mlp != 0) // down-cast worked o.k.
  {
    mlp->register_formatter ( fmt );
    return true;
  }

  return false;
}



namespace logger_ns {


// ----------------------------------------------------------------
/** \brief Constructor.
 *
 *
 */
vidtk_mini_logger_formatter
::vidtk_mini_logger_formatter()
  : m_impl(0)
{
  m_impl = new formatter_impl();
}


vidtk_mini_logger_formatter
::~vidtk_mini_logger_formatter()
{
  delete m_impl;
}


// ----------------------------------------------------------------
/** \brief Get raw implementation pointer.
 *
 *
 */
formatter_impl * vidtk_mini_logger_formatter
::get_impl()
{
  return m_impl;
}


// ----------------------------------------------------------------
/** \brief Get application instance name.
 *
 *
 */
std::string const& vidtk_mini_logger_formatter
::get_application_instance_name() const
{
  return m_impl->m_parent->get_application_instance_name();
}


// ----------------------------------------------------------------
/** \brief Get application name.
 *
 *
 */
std::string const& vidtk_mini_logger_formatter
::get_application_name() const
{
  return m_impl->m_parent->get_application_name();
}


// ----------------------------------------------------------------
/** \brief Get system name.
 *
 * This metnod returns the name of the system this program is running
 * on.  It may be blank.
 */
std::string const& vidtk_mini_logger_formatter
::get_system_name() const
{
  return m_impl->m_parent->get_system_name();
}


// ----------------------------------------------------------------
/** \brief Get logger name.
 *
 *
 */
std::string const& vidtk_mini_logger_formatter
::get_realm() const
{
  return *m_impl->m_realm;
}


// ----------------------------------------------------------------
/** \brief Get log level as a string.
 *
 * This method returns the level or severity of this log message as a
 * string.
 */
std::string vidtk_mini_logger_formatter
::get_level() const
{
  return m_impl->m_logger->get_level_string(m_impl->m_level);
}


// ----------------------------------------------------------------
/** \brief Get current process ID as string.
 *
 *
 */
std::string vidtk_mini_logger_formatter
::get_pid() const
{
  std::stringstream buf;
  buf << vpl_getpid();
  return buf.str();
}


// ----------------------------------------------------------------
/** \brief Get file name.
 *
 * This method returns the file name associated with the location of
 * this log message.
 */
std::string vidtk_mini_logger_formatter
::get_file_name() const
{
  if (m_impl->m_location)
  {
    return m_impl->m_location->get_file_name();
  }

  return "";
}


// ----------------------------------------------------------------
/** \brief Get file path.
 *
 * This method returns the file path associated with the location of
 * this log message.
 */
std::string vidtk_mini_logger_formatter
::get_file_path() const
{
  if (m_impl->m_location)
  {
    return m_impl->m_location->get_file_path();
  }

  return "";
}


// ----------------------------------------------------------------
/** \brief Get line number.
 *
 * This emthnod returns the line number of location of this log
 * message as a string. It may be blank.
 */
std::string vidtk_mini_logger_formatter
::get_line_number() const
{
  std::stringstream buf;
  if (m_impl->m_location)
  {
    buf << m_impl->m_location->get_line_number();
  }

  return buf.str();
}


// ----------------------------------------------------------------
/** \brief Get class name.
 *
 * This method returns the name of the class that generated
 * this log message.
 */
std::string vidtk_mini_logger_formatter
::get_class_name() const
{
  if (m_impl->m_location)
  {
    return m_impl->m_location->get_class_name();
  }

  return "";
}


// ----------------------------------------------------------------
/** \brief Get method name.
 *
 * This method returns the name of the method/function that generated
 * this log message.
 */
std::string vidtk_mini_logger_formatter
::get_method_name() const
{
  if (m_impl->m_location)
  {
    return m_impl->m_location->get_method_name();
  }

  return "";
}


// ----------------------------------------------------------------
/** \brief Get log message.
 *
 * This method returns the user supplied text of the log message.
 */
std::string const& vidtk_mini_logger_formatter
::get_message() const
{
  return *m_impl->m_message;
}


// ----------------------------------------------------------------
/*! \brief Get timestamp.
 *
 * This method returns the time stamp for the current log message.
 */
std::string vidtk_mini_logger_formatter
::get_time_stamp() const
{
  boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();

  std::string time_stamp(boost::posix_time::to_simple_string( now ) );
  return time_stamp;
}

} // end namespace
} // end namespace

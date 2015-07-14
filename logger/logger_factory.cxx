/*ckwg +5
 * Copyright 2010,2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <logger/logger_factory.h>

namespace vidtk {
namespace logger_ns {

// ----------------------------------------------------------------
/**
 *
 *
 */
logger_factory
::logger_factory(const char * name)
  :m_name(name)
{ }


logger_factory
:: ~logger_factory()
{ }


std::string const & logger_factory
::get_factory_name() const
{
  return m_name;
}

std::string const& logger_factory
::get_application_name() const
{
  return m_applicationName;
}


std::string const& logger_factory
::get_application_instance_name() const
{
  return m_applicationInstanceName;
}


std::string const& logger_factory
::get_system_name() const
{
  return m_systemName;
}

// ----------------------------------------------------------------
/* Set location strings.
 *
 *
 */
void logger_factory
::set_application_name (std::string const& name)
{
  m_applicationName = name;
}


void logger_factory
::set_application_instance_name (std::string const& name)
{
  m_applicationInstanceName = name;
}


void logger_factory
::set_system_name (std::string const& name)
{
  m_systemName = name;
}

} // end namespace
} // end namespace

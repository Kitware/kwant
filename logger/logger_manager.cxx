/*ckwg +5
 * Copyright 2013-2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include <logger/logger_manager.h>

#include <logger/logger_factory.h>
#include <logger/logger_factory_mini_logger.h>

#include <vul/vul_file.h>
#include <vul/vul_arg.h>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>


/*
 * Note: This must be thread safe.
 *
 * Also: In order to make this work, it must be possible to create
 * loggers before the manager has been initialized. This means that
 * the initialization is flexible, adaptive and has a reasonable
 * default.
 */

namespace vidtk {

//
// Pointer to our single instance.
//
logger_manager * logger_manager::s_instance = 0;


// ----------------------------------------------------------------
/** Constructor.
 *
 *
 */
logger_manager
::logger_manager()
  : m_logFactory(0),
    m_initialized(false)
{
  // Need to create a factory class at this point because loggers
  // are created by static initializers. we can wait no longer until
  // we provide a method for creating these loggers.

  bool try_default(false);
  char const* factory = std::getenv("VIDTK_LOGGER_FACTORY");
  if ( 0 == factory )
  {
    try_default = true;
    // If no special factory is specified, try default name
#if defined(WIN32)
    factory = "vidtk_logger_plugin.dll";
#elif defined(__APPLE__)
    factory = "vidtk_logger_plugin.dylib";
#else
    factory = "vidtk_logger_plugin.so";
#endif
  }

  try
  {
    // Dynamically load logger factory.
    m_loggerLoader.reset( new class_loader( factory ) );

    // Make sure this class is the expected logger factory.
    if ( m_loggerLoader->get_object_type() == typeid( vidtk::logger_ns::logger_factory ).name() )
    {
      // Set factory as the one loaded from library.
      m_logFactory.reset( m_loggerLoader->create_object< logger_ns::logger_factory >() );
      return;
    }
  }
  catch( std::runtime_error &e )
  {
    // Only give error if the environment specified logger could not be found
    if ( ! try_default )
    {
      std::cerr << "ERROR: Could not load logger factory as specified in environment variable \"VIDTK_LOGGER_FACTORY\"\n"
                << "Defaulting to built-in logger.\n"
                << e.what() << std::endl;
    }
    else
    {
      std::cerr << "Info: Could not load default logger factory.\n"
                << "Typical usage: export VIDTK_LOGGER_FACTORY=" << factory << "\n"
                << "Specify name of shared object, with or without a path. Behaviour depends on host system.\n"
                << "Defaulting to built-in logger." << std::endl;
    }
  }

  // Create a default logger back end
  m_logFactory.reset( new ::vidtk::logger_ns::logger_factory_mini_logger() );
}


logger_manager
::~logger_manager()
{
}


// ----------------------------------------------------------------
/** Get singleton instance.
 *
 *
 */
logger_manager * logger_manager
::instance()
{
  static boost::mutex local_lock;          // synchronization lock

  if (0 != s_instance)
  {
    return s_instance;
  }

  boost::lock_guard<boost::mutex> lock(local_lock);
  if (0 == s_instance)
  {
    // create new object
    s_instance = new logger_manager();
  }

  return s_instance;
}


// ----------------------------------------------------------------
int logger_manager
::initialize(int argc, char ** argv)
{
  std::string config_file;

  if (argc > 0)
  {
    // Get the name of the application program from the executable file name
    std::string app = vul_file::strip_directory(argv[0]);
    m_logFactory->set_application_name( app );

    // parse argv for allowable options.
    vul_arg < std::string > app_name_arg("--logger-app", "Application name. overrides argv[0].");
    vul_arg < std::string > app_instance_arg( "--logger-app-instance", "Application instance name.");
    vul_arg < std::string > config_file_arg( "--logger-config", "Configuration file name.");

    vul_arg_parse( argc, argv );

    if ( ! app_name_arg().empty())
    {
      m_logFactory->set_application_name( app_name_arg() );
    }

    m_logFactory->set_application_instance_name( app_instance_arg() );
    config_file = config_file_arg();
  }

#ifdef HAVE_GETHOSTNAME

  // Get name of the system we are running on
  ///@todo use portable call for gethostname()
  char host_buffer[1024];
  gethostname (host_buffer, sizeof host_buffer);
  m_logFactory->set_system_name( host_buffer );

#endif

  // initialise adapter to do real logging.
  m_logFactory->initialize (config_file);
  m_initialized = true;

  return (0);
}


// ----------------------------------------------------------------
/** Get address of logger object.
 *
 *
 */
vidtk_logger_sptr logger_manager
::get_logger( char const* name )
{
  return ::vidtk::logger_manager::instance()->m_logFactory->get_logger(name);
}


vidtk_logger_sptr logger_manager
::get_logger( std::string const& name )
{
  return get_logger( name.c_str() );
}


std::string const&
logger_manager
::get_factory_name() const
{
  return m_logFactory->get_factory_name();
}


  std::string const& logger_manager
::get_application_name() const
{
  return m_logFactory->get_application_name();
}


std::string const& logger_manager
::get_application_instance_name() const
{
  return m_logFactory->get_application_instance_name();
}


std::string const& logger_manager
::get_system_name() const
{
  return m_logFactory->get_system_name();
}

void logger_manager
::set_application_name( std::string const& name )
{
  m_logFactory->set_application_name( name );
}


void logger_manager
::set_application_instance_name( std::string const& name )
{
  m_logFactory->set_application_instance_name( name );
}


void logger_manager
::set_system_name( std::string const& name )
{
  m_logFactory->set_system_name( name );
}

} // end namespace

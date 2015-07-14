/*ckwg +5
 * Copyright 2010,2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <logger/logger_factory_mini_logger.h>

#include <logger/vidtk_logger_mini_logger.h>
#include <typeinfo>

#if defined LOADABLE_MODULE
// ------------------------------------------------------------------
/*
 * Shared object bootstrap function
 */
extern "C"
{
  void* class_bootstrap()
  {
    vidtk::logger_ns::logger_factory_mini_logger* ptr =  new vidtk::logger_ns::logger_factory_mini_logger;
    return ptr;
  }

  const char* class_bootstrap_type()
  {
    return typeid( vidtk::logger_ns::logger_factory ).name();
  }
}
#endif


// ------------------------------------------------------------------
namespace vidtk {
namespace logger_ns {

logger_factory_mini_logger
::logger_factory_mini_logger()
  : logger_factory("mini logger")
{

}


logger_factory_mini_logger
::~logger_factory_mini_logger()
{

}


int logger_factory_mini_logger
::initialize(std::string const& /*config_file*/)
{

  return (0);
}


// ----------------------------------------------------------------
/* Create new logger.
 *
 * A new logger object is returned with the specified name.
 */
vidtk_logger_sptr logger_factory_mini_logger
::get_logger( const char * const name )
{
  return new vidtk_logger_mini_logger( this, name);
}

} // end namespace
} // end namespace

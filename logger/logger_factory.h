/*ckwg +5
 * Copyright 2010,2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */



#ifndef _LOGGER_NS_LOGGER_FACTORY_H_
#define _LOGGER_NS_LOGGER_FACTORY_H_

#include <logger/vidtk_logger.h>

#include <string>
#include <boost/noncopyable.hpp>


namespace vidtk {
namespace logger_ns {

// ----------------------------------------------------------------
/** Factory for underlying logger.
 *
 * This class is the abstract base class that adapts the VIDTK logger
 * to the underlying logging system.
 *
 * An object of this type can be created early in the program
 * execution (i.e. static initializer time), which is before the
 * initialize method is called.
 */
class logger_factory
  : private boost::noncopyable
{
public:
  /**
   * @brief Create logger factory
   *
   * The name for this factory should describe the logger type that is
   * being created.
   *
   * Call get_name() to access this name.
   *
   * @param name  Name of this logger factory
   */
  logger_factory( const char* name );
  virtual ~logger_factory();

  /**
   * @brief Initialize the logger factory.
   *
   * This method passes a config file to the derived logger
   * factory. This file could contain anything that is needed to
   * configure the underlying lager provider.
   *
   * @param config_file Name of configuration file.
   *
   * @return 0 if all went well.
   */
  virtual int initialize( std::string const& config_file ) = 0;

  /**
   * @brief Get pointer to named logger.
   *
   * This method returns a pointer to a named logger. The underlying
   * log provider determines what is actually done here.
   *
   * @param name Name of logger object.
   *
   * @return
   */
  virtual vidtk_logger_sptr get_logger( const char* const name ) = 0;
  vidtk_logger_sptr get_logger( std::string const& name )
  {  return get_logger( name.c_str() ); }

  /**
   * @brief Get logger factory name.
   *
   * Returns the name associated with this logger factory.
   *
   * @return Name or type of logger created by this factory.
   */
  std::string const& get_factory_name() const;

  /**
   * @brief Get name of application.
   *
   * This method returns the name of the application. This name is set
   * when the factory is created or can be updated later.
   *
   * The application_name is the name given to the whole logical
   * application. It may be a distributed application.
   *
   * @return Application name.
   */
  std::string const& get_application_name() const;

  /**
   * @brief Get name of application instance.
   *
   * This method returns the instance name of the application. This
   * name is set when the factory is created or can be updated later.
   *
   * The application instance name uniquely identifies an instance of
   * an application. This is especially useful when there is more than
   * one instance if an applications.
   *
   * @return Name of instance of application.
   */
  std::string const& get_application_instance_name() const;

  /**
   * @brief Get host name.
   *
   * This method returns the name of the system the application is
   * running on. This is useful for distributed applications.
   *
   * @return Name of system
   */
  std::string const& get_system_name() const;

  // -- MANIPULATORS --
  void set_application_name( std::string const& name );
  void set_application_instance_name( std::string const& name );
  void set_system_name( std::string const& name );


private:
  std::string m_name; // factory name

  // location of this log manager
  std::string m_applicationName;
  std::string m_applicationInstanceName;
  std::string m_systemName;
  // TODO   <ip-addr> m_systemIPAddr;

}; // end class logger_factory

} // end namespace
} // end namespace

#endif /* _LOGGER_NS_LOGGER_FACTORY_H_ */

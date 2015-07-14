/*ckwg +5
 * Copyright 2010-2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#ifndef _LOGGER_MANAGER_H_
#define _LOGGER_MANAGER_H_

#include <logger/vidtk_logger.h>

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <logger/class_loader.h>

namespace vidtk {

//
// Partial types
//
namespace logger_ns {  class logger_factory; }
class vidtk_logger;


// ----------------------------------------------------------------
/** Logger manager (root object)
 *
 * This class represents the main top level logic for the VIDTK
 * logger. Only one object of this type is required, so this is a
 * singleton created by the static instance() method.
 *
 * The manager can handle get_logger() calls before it it is
 * initialized. Specifically, it is quite likely that get_logger()
 * calls will be made before the initialize() call.
 */
class logger_manager
  : public boost::noncopyable
{
public:
  virtual ~logger_manager();

  /** Get the single instance of this class. */
  static logger_manager * instance();

/** Initialize.
 *
 * Initialize the logging subsystem using the command line
 * arguments.
 *
 * The following items are initialized:
 *
 * --logger-app <name> - specifies the application name. Overrides the name in argv[0].<br>
 * --logger-app-instance <name> - specifies the aplication isntance name. This is useful
 * in multi-process applications where there is more than one instance of a program.<br>
 * --logger-config <name> - specifies the name of the logger configuration file.
 * The syntax and semantics of this file depends on which logger factory is active.<br>
 *
 * If no parameters are passed to the initialize() method, then the
 * application name and instance name will not be available to the
 * logger.
 *
 * The configuration file structure and syntax depends on the
 * underlying logger implementaion currently being used.
 *
 * @param argc  number of elements in argv,
 * @param argv  vector of erguments.
 *
 * @retval 0 - initialization completed o.k.
 * @retval -1 - error in initialization.

 */
  int initialize(int argc = 0, char * argv[] = 0);

  /** Get named logger object. */
  static vidtk_logger_sptr get_logger( char const* name );

  /** Get named logger object. */
  static vidtk_logger_sptr get_logger( std::string const& name );

  /**
   * @brief Get name of current logger factory.
   *
   * @return Name of logger factory.
   */
  std::string const&  get_factory_name() const;

  std::string const& get_application_name() const;
  std::string const& get_application_instance_name() const;
  std::string const& get_system_name() const;

  // -- MANIPULATORS --
  void set_application_name( std::string const& name );
  void set_application_instance_name( std::string const& name );
  void set_system_name( std::string const& name );

private:
  logger_manager();

  boost::scoped_ptr< logger_ns::logger_factory > m_logFactory;
  boost::scoped_ptr< class_loader > m_loggerLoader;

  bool m_initialized;

  static logger_manager * s_instance;
}; // end class logger_manager

} // end namespace


#endif /* _LOGGER_MANAGER_H_ */

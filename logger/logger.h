/*ckwg +5
 * Copyright 2010-2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef _LOGGER__API_H_
#define _LOGGER__API_H_

#include <logger/logger_manager.h>
#include <logger/location_info.h>
#include <sstream>
#include <cstdlib>

// ----------------------------------------------------------------
/** @page Logger Logger Documentation

<P>The vidtk logger (class vidtk_logger) provides a interface to an
underlying log implementation. Log4cxx is the baseline implementation,
which is why this interface looks the way it does. Alternate loggers
can be instantiated as needed for specific applications, products, or
projects. These alternate logger inplementations are supplied by a
factory class and can provide any functionality consistent with the
vidtk_logger interface.</P>

<P>The logger_manager class is a singleton and is accessed through the
vidtk::logger_manager::instance() pointer. Individual logger objects
(derived from vidtk_logger class) can be created before the
logger_manager is initialized, but not used.  This allows loggers to
be created by static initializers, but be sure to initialize the
log_manager as soon as practical in the application, since the logger
will fail if log messages are generated before initialization.</P>

<P>The easiest way to applying the logger is to use the macros in the
logger/logger.h file. The \c VIDTK_LOGGER("name"); macro creates a \b
static logger with the specified name that is available within its
enclosing scope and is used by the \c LOG_*() macros.  These log
macros automatically determine the source file location (file name,
line number, method name) and make it available to the logger.</p>

@sa vidtk::vidtk_logger


<h2>Internal Operation</h2>

<p>During construction, the logger_manager instantiates a factory class
based on the selected underlying logger.  This factory class creates
logger (vidtl_logger) objects for use by the application. Creating a
logger in the static constructor phase will instantiate the
logger_manager.</p>

<P>The ability to support alternate underlying logger implementations
is designed to make this logging component easy(er) to transport to
projects that have a specific (not log4cxx) logging implementation
requirement. Alternate logger factories are dynamically loaded at run
time.</p>

<P>When the logger manager starts, it looks for a default logger
implementation by attempting to load a module maned
"vidtk_logger_plugin.so" (or .dll for windows). If the default plugin
is not found, then the environment variable "VIDTK_LOGGER_FACTORY" is
checked. If it is present, the manager attempts to load it as the
logger plugin. If the environment variable is not present or the load
fails, then the default mini-logger is used.</p>

<p>Using a default plugin allows a logger factory to be supplied by
the installed set of libraries. This helps in cases where it is not
practical to set an environment variable. A log4cxx logger factory is
built by default and can be set up as the default plugin by renaming
it or using a symbolic link.</p>

<h2>Configuration</h2>

At a minimum, the logger does not require any external configuration
to operate, but there are cases (e.g. when an application wants to
customize the logger) that need more control over the logger.  The
logger manager can be configured by calling the
logger_manager::initialize( argc, argv ) method. This method should be
called before any log messages are issued.

The main function \c argv and \c argc parameters are passed into the
logger initialization method. This method looks for the following
optional parameters from the command line. \n

--logger-app <name> - specifies the application name. Overrides the name in argv[0].<br>
--logger-app-instance <name> - specifies the application instance name. This is useful
in multi-process applications where there is more than one instance of a program.<br>
--logger-config <name> - specifies the name of the logger
configuration file. This file name is passed to the logger
factory.  The syntax and semantics of this file depends on which
logger factory is active.<br>

For the log4cxx version --

If no configuration file command line parameter is specified, the
environment variable "LOG4CXX_CONFIGURATION" is checked for the
configuration file name.

If a configuration file has not been specified, as above, the logger
looks for the file "log4cxx.properties" in the current directory.

If still no configuration file can be found, then a default
configuration is used, which generally does not do what you really
want.

<h2>Example</h2>

\code
#include <logger/logger.h>
#include <iostream>

// Create static default logger for log macros
VIDTK_LOGGER("main-test");

int main(int argc, char *argv[])
{

  // Initialize the logger component from command line parameters.
  vidtk::logger_manager::instance()->initialize(argc, argv);

  LOG_ERROR("first message" << " from here");

  LOG_ASSERT( false, "assertion blown" ); (logged at FATAL level)
  LOG_FATAL("fatal message");
  LOG_ERROR("error message");
  LOG_WARN ("warning message");
  LOG_INFO ("info message");
  LOG_DEBUG("debug message");
  LOG_TRACE("trace message");

  // A logger can be explicitly created if needed.
  vidtk_logger_sptr log2 =  logger_manager::instance()->get_logger("main.logger2");

  log2->set_level(vidtk_logger::LEVEL_WARN);

  std::cout << "Current log level "
           << log2->get_level_string (log2->get_level())
           << std::endl;

  log2->log_fatal("direct logger call");
  log2->log_error("direct logger call");
  log2->log_warn ("direct logger call");
  log2->log_info ("direct logger call");
  log2->log_debug("direct logger call");
  log2->log_trace("direct logger call");

  return 0;
}
\endcode

 */

#define VIDTK_DEFAULT_LOGGER __vidtk_logger__


// ----------------------------------------------------------------
/** Instantiate a named logger.
 *
 *
 */
#define VIDTK_LOGGER(NAME) static ::vidtk::vidtk_logger_sptr VIDTK_DEFAULT_LOGGER = \
    ::vidtk::logger_manager::instance()->get_logger("vidtk."  NAME)


/**
Logs a message with the FATAL level. The logger defined by the
VIDTK_LOGGER macro is used to process the log message.

Even though you may think that a FATAL error would terminate the
program, we are only the logger and should not presume to alter
control flow in such a radical manner. Also, any other behaviour would
be inconsistent with the other macros that just log a message at the
specified level.

If you need terminal behaviour, use LOG_AND_DIE().

@sa LOG_AND_DIE
@param msg the message string to log.
*/
#define LOG_FATAL(msg) do {                                             \
if (VIDTK_DEFAULT_LOGGER->is_fatal_enabled()) {                         \
  std::stringstream _oss_; _oss_ << msg;                                \
  VIDTK_DEFAULT_LOGGER->log_fatal(_oss_.str(), VIDTK_LOGGER_SITE); }    \
} while(0)


/**
Logs a message with the FATAL level and terminate the program. The
logger defined by the VIDTK_LOGGER macro is used to process the log
message.

@param msg the message string to log.
*/
#define LOG_AND_DIE(msg) do {                                           \
  std::stringstream _oss_; _oss_ << msg;                                \
  VIDTK_DEFAULT_LOGGER->log_fatal(_oss_.str(), VIDTK_LOGGER_SITE);      \
  std::exit(1);                                                         \
} while(0)


/**
Performs assert and logs message if condition is false.  If condition
is false, log a message at the FATAL level is generated. This is
similar to the library assert except that the message goes to the
logger.

@param cond the condition which should be met to pass the assertion
@param msg the message string to log.
*/
#ifndef NDEBUG

#define LOG_ASSERT(cond,msg) do {                                       \
  if (!(cond) ) {                                                       \
  std::stringstream _oss_; _oss_ << "ASSERTION FAILED: ("               \
  << # cond ")\n"  << msg;                                              \
  VIDTK_DEFAULT_LOGGER->log_fatal(_oss_.str(), VIDTK_LOGGER_SITE);      \
  std::abort(); }                                                       \
} while(0)

#else

#define LOG_ASSERT(cond,msg) do {               \
  if (!(cond) ) {                               \
  }                                             \
} while(0)

#endif


/**
Logs a message with the ERROR level. The logger defined by the
VIDTK_LOGGER macro is used to process the log message.

@param msg the message string to log.
*/
#define LOG_ERROR(msg) do {                                             \
if (VIDTK_DEFAULT_LOGGER->is_error_enabled()) {                         \
std::stringstream _oss_; _oss_ << msg;                                  \
  VIDTK_DEFAULT_LOGGER->log_error(_oss_.str(), VIDTK_LOGGER_SITE); }    \
} while(0)


/**
Logs a message with the WARN level. The logger defined by the
VIDTK_LOGGER macro is used to process the log message.

@param msg the message string to log.
*/
#define LOG_WARN(msg) do {                                              \
if (VIDTK_DEFAULT_LOGGER->is_warn_enabled()) {                          \
std::stringstream _oss_; _oss_ << msg;                                  \
  VIDTK_DEFAULT_LOGGER->log_warn(_oss_.str(), VIDTK_LOGGER_SITE); }     \
} while(0)


/**
Logs a message with the INFO level. The logger defined by the
VIDTK_LOGGER macro is used to process the log message.

@param msg the message string to log.
*/
#define LOG_INFO(msg) do {                                              \
if (VIDTK_DEFAULT_LOGGER->is_info_enabled()) {                          \
std::stringstream _oss_; _oss_ << msg;                                  \
  VIDTK_DEFAULT_LOGGER->log_info(_oss_.str(), VIDTK_LOGGER_SITE); }     \
} while(0)


/**
Logs a message with the DEBUG level. The logger defined by the
VIDTK_LOGGER macro is used to process the log message.

@param msg the message string to log.
*/
#define LOG_DEBUG(msg) do {                                             \
if (VIDTK_DEFAULT_LOGGER->is_debug_enabled()) {                         \
std::stringstream _oss_; _oss_ << msg;                                  \
  VIDTK_DEFAULT_LOGGER->log_debug(_oss_.str(), VIDTK_LOGGER_SITE); }    \
} while(0)


/**
Logs a message with the TRACE level. The logger defined by the
VIDTK_LOGGER macro is used to process the log message.

@param msg the message string to log.
*/
#define LOG_TRACE(msg) do {                                             \
if (VIDTK_DEFAULT_LOGGER->is_trace_enabled()) {                         \
std::stringstream _oss_; _oss_ << msg;                                  \
  VIDTK_DEFAULT_LOGGER->log_trace(_oss_.str(), VIDTK_LOGGER_SITE); }    \
} while(0)

// Test for debugging level being enabled
#define IS_FATAL_ENABLED() (VIDTK_DEFAULT_LOGGER->is_fatal_enabled())
#define IS_ERROR_ENABLED() (VIDTK_DEFAULT_LOGGER->is_error_enabled())
#define IS_WARN_ENABLED()  (VIDTK_DEFAULT_LOGGER->is_warn_enabled())
#define IS_INFO_ENABLED()  (VIDTK_DEFAULT_LOGGER->is_info_enabled())
#define IS_DEBUG_ENABLED() (VIDTK_DEFAULT_LOGGER->is_debug_enabled())
#define IS_TRACE_ENABLED() (VIDTK_DEFAULT_LOGGER->is_trace_enabled())


#endif /* _LOGGER__API_H_ */

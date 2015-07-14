/*ckwg +5
 * Copyright 2010-2012,2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */



#include <logger/logger.h>
#include <iostream>
#include <logger/vidtk_mini_logger_formatter.h>
#include <logger/vidtk_logger_mini_logger.h>


VIDTK_LOGGER("main.logger");

using namespace vidtk;


// ----------------------------------------------------------------
/** Basic formatter for mini logger.
 *
 * Log messages are formatted in a very plain manner, with just the
 * log level and message text. A new line is appended to the message
 * in keeping with standard logging practice of not including a
 * trailing newline in the message.
 *
 * This class can be considered a starting point for writing a custom
 * formatting class. Make a copy and rename, do not derive from this
 * class.
 */
class vidtk_mini_logger_formatter_test
  : public ::vidtk::logger_ns::vidtk_mini_logger_formatter
{
public:
  vidtk_mini_logger_formatter_test() { }
  virtual ~vidtk_mini_logger_formatter_test() { }

  virtual void format_message(std::ostream& str);

}; // end class vidtk_mini_logger_formatter_test

void vidtk_mini_logger_formatter_test::
format_message(std::ostream& str)
{
  // Format this message on the stream
  str << get_level() << " "

      << "[";

  if ( ! get_application_instance_name().empty() )
  {
    str << get_application_instance_name() << ":";
  }

  str << get_application_name()
      << ":" << get_realm()
      << "(" << get_pid() << ")";

  if ( ! get_system_name().empty() )
  {
    str << " @ " << get_system_name();
  }

  str << "] ";

  // test for location information available
  if (get_file_name() != "")
  {
    str << get_file_name()
        << ":" << get_line_number()
        << "(" << get_method_name() << "())";
  }

  str << "- "
      << get_message()
      << std::endl;
}



// ----------------------------------------------------------------
int main(int argc, char *argv[])
{

  vidtk::logger_manager::instance()->initialize(argc, argv);

  std::cout << "Logger using factory: "
            << vidtk::logger_manager::instance()->get_factory_name()
            << std::endl;

  // TEST macros
  std::cout << "\nTesting log macros. The log lines will print twice when using log4cxx unless\n"
           << "there is a log4cxx.properties file specifying an appender.\n"
           << "That just the way it is!\n"
           << std::endl;

  LOG_ERROR("MACRO first message" << " from here");

  LOG_FATAL("MACRO fatal message");
  LOG_ERROR("MACRO error message");
  LOG_WARN ("MACRO warning message");
  LOG_INFO ("MACRO info message");
  LOG_DEBUG("MACRO debug message");
  LOG_TRACE("MACRO trace message");

  vidtk_logger_sptr log2 =  logger_manager::instance()->get_logger("main.logger2");

  // Test setting a new formatter. As a side effect, we loose some
  // memory if not the mini-logger. (All in the name of science.)
  bool rc = set_mini_logger_formatter (log2, new vidtk_mini_logger_formatter_test() );
  if (rc)
  {
    std::cout << "Successfully set new formatter\n";
  }
  else
  {
    std::cout << "Cound not set new formatter - wrong logger type\n";
  }

  std::cout << "\nUsing another logger.\n";

  log2->set_level(vidtk_logger::LEVEL_WARN);

  std::cout << "\nCurrent log level "
           << log2->get_level_string (log2->get_level())
           << std::endl << std::endl;

  // TEST direct call
  log2->log_fatal("direct logger call");
  log2->log_error("direct logger call");
  log2->log_warn("direct logger call");
  log2->log_info("direct logger call");
  log2->log_debug("direct logger call");
  log2->log_trace("direct logger call");

  log2->set_level(vidtk_logger::LEVEL_TRACE);

  std::cout << "\nCurrent log level "
           << log2->get_level_string (log2->get_level())
           << std::endl << std::endl;

  // TEST direct call
  log2->log_fatal("direct logger call");
  log2->log_error("direct logger call");
  log2->log_warn("direct logger call");
  log2->log_info("direct logger call");
  log2->log_debug("direct logger call");
  log2->log_trace("direct logger call");

  return 0;
}

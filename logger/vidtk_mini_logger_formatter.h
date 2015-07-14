/*ckwg +5
 * Copyright 2010-2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#ifndef _VIDTK_MINI_LOGGER_FORMATTER_H_
#define _VIDTK_MINI_LOGGER_FORMATTER_H_

#include <string>
#include <ostream>

namespace vidtk {
namespace logger_ns {

class formatter_impl;

// ----------------------------------------------------------------
/** Base formatter class
 *
 * This class represents the base class for formatting log messages.
 */
class vidtk_mini_logger_formatter
{
public:
  vidtk_mini_logger_formatter();
  virtual ~vidtk_mini_logger_formatter();

  virtual void format_message(std::ostream& str) = 0;

  formatter_impl * get_impl();


protected:

  std::string const& get_application_instance_name() const;
  std::string const& get_application_name() const;
  std::string const& get_system_name() const;
  std::string const& get_realm() const;
  std::string get_level() const;
  std::string get_pid() const;
  virtual std::string get_time_stamp() const;

  std::string get_file_name() const;
  std::string get_file_path() const;
  std::string get_line_number() const;
  std::string get_class_name() const;
  std::string get_method_name() const;

  std::string const& get_message() const;

private:
  formatter_impl * m_impl;


}; // end class vidtk_mini_logger_formatter

} // end namespace
} // end namespace


#endif /* _VIDTK_MINI_LOGGER_FORMATTER_H_ */

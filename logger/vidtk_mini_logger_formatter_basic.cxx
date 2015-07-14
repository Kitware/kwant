/*ckwg +5
 * Copyright 2010-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include <logger/vidtk_mini_logger_formatter_basic.h>


namespace vidtk {
namespace logger_ns {

vidtk_mini_logger_formatter_basic
::vidtk_mini_logger_formatter_basic()
{

}


vidtk_mini_logger_formatter_basic
::~vidtk_mini_logger_formatter_basic()
{

}

void vidtk_mini_logger_formatter_basic
::format_message(std::ostream& str)
{
  str << get_time_stamp() << " " << get_level() << " "

#if 0

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

  str << "] "

#endif

      << get_file_name() << ":" << get_line_number()
      << "(" << get_method_name() << "())"
      << " - "
      << get_message()
      << std::endl;
}


} // end namespace
} // end namespace

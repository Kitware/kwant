/*ckwg +5
 * Copyright 2011-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include <logger/vidtk_mini_logger_formatter_json.h>

namespace vidtk {
namespace logger_ns {

vidtk_mini_logger_formatter_json
::vidtk_mini_logger_formatter_json()
{

}


vidtk_mini_logger_formatter_json
::~vidtk_mini_logger_formatter_json()
{

}

void vidtk_mini_logger_formatter_json
::format_message(std::ostream& str)
{
  str << "{ \"log_entry\" : {" << std::endl
      << "    \"system_name\" : \"" << get_system_name() << "\"," << std::endl
      << "    \"application\" : \"" << get_application_name() << "\"," << std::endl
      << "    \"application_instance\" : \"" << get_application_instance_name() << "\"," << std::endl
      << "    \"logger\" : \"" << get_realm() << "\"," << std::endl
      << "    \"pid\" : \"" << get_pid() << "\"," << std::endl
      << "    \"level\" : \"" << get_level() << "\"," << std::endl
      << "    \"path\" : \"" << get_file_path() << "\"," << std::endl
      << "    \"time_stamp\" : \"" << get_time_stamp() << "\"," << std::endl
      << "    \"filename\" : \"" << get_file_name() << "\"," << std::endl
      << "    \"line_number\" : " << get_line_number() << "," << std::endl
      << "    \"class_name\" : \"" << get_class_name() << "\"," << std::endl
      << "    \"method_name\" : \"" << get_method_name() << "\"," << std::endl
      << "    \"message\" : \"" << get_message() << "\"" << std::endl
      << "} }"  << std::endl;
}


} // end namespace
} // end namespace

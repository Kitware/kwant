/*ckwg +5
 * Copyright 2010-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#ifndef _VIDTK_MINI_LOGGER_FORMATTER_JSON_H_
#define _VIDTK_MINI_LOGGER_FORMATTER_JSON_H_

#include <logger/vidtk_mini_logger_formatter.h>

namespace vidtk {
namespace logger_ns {

// ----------------------------------------------------------------
/** Basic formatter for mini logger.
 *
 * Log messages are formatted as JSON strings. Each log message is
 * formatted as a single JSON structure with the following format:
 *
 * log_entry : {<br>
 *         system_name :<br>
 *         application :<br>
 *         application_instance :<br>
 *         logger :<br>
 *         pid :<br>
 *         level :<br>
 *         path :<br>
 *         filename :<br>
 *         line_number :<br>
 *         class_name :<br>
 *         method_name :<br>
 *         message :<br>
 *     }<br>
 *
 * This formatter can be used with a logger as shown in the following
 * example:
 *
 * \code

 // create a new logger
 vidtk::vidtk_logger_sptr log =  vidtk::logger_manager::instance()->get_logger("json_logger");

 // Create new JSON formatter
 vidtk::logger_ns::vidtk_mini_logger_formatter_json * jfmt(new vidtk::logger_ns::vidtk_mini_logger_formatter_json() );

 // Set as the logger formatter. Only works for minilogger implementations.
 if ( ! set_mini_logger_formatter (log, jfmt ))
{
   LOG_ERROR("Could not set JSON formatter for this logger");
}

 * \endcode
 */
class vidtk_mini_logger_formatter_json
  : public vidtk_mini_logger_formatter
{
public:
  vidtk_mini_logger_formatter_json();
  virtual ~vidtk_mini_logger_formatter_json();

  virtual void format_message(std::ostream& str);

}; // end class vidtk_mini_logger_formatter_json

} // end namespace
} // end namespace

#endif /* _VIDTK_MINI_LOGGER_FORMATTER_JSON_H_ */

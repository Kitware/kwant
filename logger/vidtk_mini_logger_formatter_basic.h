/*ckwg +5
 * Copyright 2010-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#ifndef _VIDTK_MINI_LOGGER_FORMATTER_BASIC_H_
#define _VIDTK_MINI_LOGGER_FORMATTER_BASIC_H_

#include <logger/vidtk_mini_logger_formatter.h>

namespace vidtk {
namespace logger_ns {

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
class vidtk_mini_logger_formatter_basic
  : public vidtk_mini_logger_formatter
{
public:
  vidtk_mini_logger_formatter_basic();
  virtual ~vidtk_mini_logger_formatter_basic();

  virtual void format_message(std::ostream& str);

}; // end class vidtk_mini_logger_formatter_basic

} // end namespace
} // end namespace

#endif /* _VIDTK_MINI_LOGGER_FORMATTER_BASIC_H_ */

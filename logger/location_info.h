/*ckwg +5
 * Copyright 2010,2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#ifndef _LOCATION_INFO_H_
#define _LOCATION_INFO_H_

#include <string>


namespace vidtk {
namespace logger_ns {


// ----------------------------------------------------------------
/** Location of logging call.
 *
 * This class represents the location of the logging call.
 *
 */
class location_info
{
public:
  /** Constructor. Create a default of unknown location */
  location_info();

  /** Constructor. Create a location object for the current site */
  location_info( char const* filename, char const* method, int line );

  //@{
  /** Default values for unknown locations */
  static const char * const NA;
  static const char * const NA_METHOD;
  //@}

  /** Get file name. The file name for the current location is
   * returned.
   * @return file name, may be null. */
  std::string get_file_name() const;
  char const * get_file_name_ptr() const { return m_fileName; }

  /** Get path part of file spec. The path or base name portion of the
   * file path is returned.
   * @ return file name base. May be null. */
  std::string get_file_path() const;

  /** Get full function/method signature. The whole signature, as
   * captured by the macro, is returned.
   */
  std::string get_signature() const;

  /** Get method name. The method name for the current location is
   * returned.
   * @return method name, may be null. */
  std::string get_method_name() const;
  char const * get_method_name_ptr() const { return m_methodName; }

  /** Get class name. This method returns the method name for the
   * current location
   * @return class name. */
  std::string get_class_name() const;

  /** Get line number. The line number for the current location is
   * returned.
   * @return line number, -1 indicates unknown line. */
  int get_line_number() const;


private:
  const char * const m_fileName;
  const char * const m_methodName;
  int m_lineNumber;

}; // end class location_info

} // end namespace
} // end namespace


#if defined(_MSC_VER)
#if _MSC_VER >= 1300
      #define __VIDTK_LOGGER_FUNC__ __FUNCSIG__
#endif
#else
#if defined(__GNUC__)
      #define __VIDTK_LOGGER_FUNC__ __PRETTY_FUNCTION__
#endif
#endif
#if !defined(__VIDTK_LOGGER_FUNC__)
#define __VIDTK_LOGGER_FUNC__ ""
#endif

#define VIDTK_LOGGER_SITE ::vidtk::logger_ns::location_info(__FILE__,   \
           __VIDTK_LOGGER_FUNC__,                                       \
           __LINE__)

#endif /* _LOCATION_INFO_H_ */

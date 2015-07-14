/*ckwg +5
 * Copyright 2010,2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include <logger/location_info.h>

#include <vpl/vpl.h>
#include <vul/vul_file.h>


namespace vidtk {
namespace logger_ns {

/**
   When location information is not available the constant
   <code>NA</code> is returned. Current value of this string
   constant is <b>?</b>.  */
const char* const location_info::NA = "?";
const char* const location_info::NA_METHOD = "?::?";


// ----------------------------------------------------------------
/** Constructor.
 *
 * The default constructor creates a location with all fields set to
 * the "unknown" state.
 */
location_info
::location_info()
  : m_fileName(location_info::NA),
    m_methodName(location_info::NA_METHOD),
    m_lineNumber(-1)
{

}


// ----------------------------------------------------------------
/** Constructor.
 *
 * This constructor creates a location object with a fully described
 * location.
 */
location_info
::location_info (char const* filename, char const* method, int line )
  : m_fileName(filename),
    m_methodName(method),
    m_lineNumber(line)
{

}


// ----------------------------------------------------------------
/** Get file name.
 *
 *
 */
std::string location_info
::get_file_name() const
{
  return vul_file::strip_directory (m_fileName);
}


// ----------------------------------------------------------------
/** Get file path.
 *
 *
 */
std::string location_info
::get_file_path() const
{
  return vul_file::basename (m_fileName);
}


// ----------------------------------------------------------------
/** Get method/function signature.
 *
 *
 */
std::string location_info
::get_signature() const
{
  return m_methodName;
}


// ----------------------------------------------------------------
/** Get method name.
 *
 * This method parses the signature string for the method name.
 * ex: void namespace::class::method(class::type var)
 * would return "method"
 */
std::string location_info
::get_method_name() const
{
  std::string tmp(m_methodName);

  // Clear all parameters from signature
  size_t parenPos = tmp.find('(');
  if (parenPos != std::string::npos)
  {
    tmp.erase(parenPos);
  }

  size_t colonPos = tmp.rfind("::");
  if (colonPos != std::string::npos)
  {
    tmp.erase(0, colonPos + 2);
  }

  size_t spacePos = tmp.rfind(' ');
  if (spacePos != std::string::npos)
  {
    tmp.erase(0, spacePos + 1);
  }

  return ( tmp );
}


// ----------------------------------------------------------------
/** Get class name.
 *
 * This method parses the signature string for the method name.
 * ex: void namespace::class::method(class::type var)
 * would return "namespace::class"
 */
std::string location_info
::get_class_name() const
{
  std::string tmp(m_methodName);

  // Clear all parameters from signature
  size_t parenPos = tmp.find('(');
  if (parenPos != std::string::npos)
  {
    tmp.erase(parenPos);
  }

  // Erase return type if any
  size_t spacePos = tmp.rfind(' ');
  if (spacePos != std::string::npos)
  {
    tmp.erase(0, spacePos + 1);
  }

  // erase all characters after last "::"
  size_t colonPos = tmp.rfind("::");
  if (colonPos != std::string::npos)
  {
    tmp.erase(colonPos);
  }
  else
  {
    // no class if no "::"
    tmp.clear();
  }

  return ( tmp );
}


// ----------------------------------------------------------------
/** Get line number.
 *
 *
 */
int location_info
::get_line_number() const
{
  return m_lineNumber;
}


} // end namespace
} // end namespace

/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_EXCEPTIONS_H_
#define VIDTK_LIBRARY_STANAG_4676_EXCEPTIONS_H_

#include <exception>
#include <string>

#if __cplusplus >= 201103L
#define STANAG_4676_NOEXCEPT noexcept
#else
#define STANAG_4676_NOEXCEPT throw()
#endif

class TiXmlElement;

namespace STANAG_4676
{

class Exception : public std::exception
{
public:
  Exception(void);
  virtual ~Exception() STANAG_4676_NOEXCEPT;
  virtual const char* what(void) const STANAG_4676_NOEXCEPT;

protected:
  std::string _what;
};

class MissingTypeException : public Exception
{
public:
  MissingTypeException(const TiXmlElement* elem);
  virtual ~MissingTypeException() STANAG_4676_NOEXCEPT;
};

class UnsupportedTypeException : public Exception
{
public:
  UnsupportedTypeException(const TiXmlElement* elem, const std::string& type);
  virtual ~UnsupportedTypeException() STANAG_4676_NOEXCEPT;
};

class MissingChildException : public Exception
{
public:
  MissingChildException(const TiXmlElement* elem, const std::string& name);
  MissingChildException(const TiXmlElement* elem, const std::string& name,
                        size_t min_occurs, size_t count);
  virtual ~MissingChildException() STANAG_4676_NOEXCEPT;
};

class ParseException : public Exception
{
public:
  ParseException(const TiXmlElement* elem, const TiXmlElement* parent = 0);
  virtual ~ParseException() STANAG_4676_NOEXCEPT;
};

} // End STANAG_4676 namespace
#endif

/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "Exceptions.h"

#include <tinyxml.h>

#include <sstream>

class TiXmlElement;

namespace STANAG_4676
{

Exception
::Exception(void)
{
}

Exception
::~Exception() STANAG_4676_NOEXCEPT
{
}

const char*
Exception
::what(void) const STANAG_4676_NOEXCEPT
{
  return this->_what.c_str();
}

MissingTypeException::MissingTypeException(const TiXmlElement* elem)
{
  std::ostringstream oss;
  oss << "XML node \'" << elem->ValueStr()
      << "\' requires a type but did not specify one";
  this->_what = oss.str();
}

MissingTypeException
::~MissingTypeException() STANAG_4676_NOEXCEPT
{
}

UnsupportedTypeException
::UnsupportedTypeException(const TiXmlElement* elem, const std::string& type)
{
  std::ostringstream oss;
  oss << "XML node \'" << elem->ValueStr()
      << "\' has unsupported type \'" << type << "\'";
  this->_what = oss.str();
}

UnsupportedTypeException
::~UnsupportedTypeException() STANAG_4676_NOEXCEPT
{
}

MissingChildException
::MissingChildException(const TiXmlElement* elem, const std::string& name)
{
  std::ostringstream oss;
  oss << "XML node \'" << elem->ValueStr()
      << "\' missing required value (child element) \'" << name << "\'";
  this->_what = oss.str();
}

MissingChildException
::MissingChildException(const TiXmlElement* elem, const std::string& name,
                        size_t min_occurs, size_t count)
{
  std::ostringstream oss;
  oss << "XML node \'" << elem->ValueStr()
      << "\' requires at least " << min_occurs << " values (child elements) \""
      << name << "\', but " << count << " were [successfully] read";
  this->_what = oss.str();
}

MissingChildException
::~MissingChildException() STANAG_4676_NOEXCEPT
{
}

ParseException
::ParseException(const TiXmlElement* elem, const TiXmlElement* parent)
{
  std::ostringstream oss;
  oss << "Error parsing XML node \'" << elem->ValueStr() << "\'";
  if (parent)
    oss << " (from parent XML node \'" << parent->ValueStr() << "\')";
  this->_what = oss.str();
}

ParseException
::~ParseException() STANAG_4676_NOEXCEPT
{
}

} // End STANAG_4676 namespace

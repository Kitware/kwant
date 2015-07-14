/*ckwg +5
 * Copyright 2013-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "XMLIOBase.h"
#include <vbl/vbl_smart_ptr.txx>
#include <boost/regex.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>
#include <iomanip>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::XMLIOBase);

#include <logger/logger.h>
#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __STANAG_4676_XMLIOBASE__
VIDTK_LOGGER("STANAG_4676_XMLIOBase");

namespace STANAG_4676
{

XMLIOBase::XMLIOBase(void)
  : _error_count(0)
{
}

// Helper functions for XML conversions

TiXmlElement*
XMLIOBase
::toXML(const std::string& name, const std::string& value)
{
  TiXmlElement* e = new TiXmlElement(name);
  e->LinkEndChild(new TiXmlText(value));
  return e;
}

TiXmlElement*
XMLIOBase
::toXML(const std::string& name, double value)
{
  std::stringstream ss;
  ss << std::fixed << std::setprecision(16) << value;
  return toXML(name, ss.str());
}

TiXmlElement*
XMLIOBase
::toXML(const std::string& name, int value)
{
  std::stringstream ss;
  ss << value;
  return toXML(name, ss.str());
}

TiXmlElement*
XMLIOBase
::toXML(const std::string& name, const boost::uuids::uuid& value)
{
  std::stringstream ss;
  ss << value;
  return toXML(name, ss.str());
}

TiXmlElement*
XMLIOBase
::toXML(const std::string& name, const boost::posix_time::ptime& value)
{
  using namespace boost::posix_time;
  std::stringstream ss;
  time_facet* output_facet = new time_facet;
  std::locale special_locale(std::locale( "C" ), output_facet);
  ss.imbue(special_locale);
  output_facet->format("%Y-%m-%dT%H:%M:%sZ");
  ss << value;
  return toXML(name, ss.str());
}

double
XMLIOBase
::wrapToRange(double value, double min, double max, double span)
{
  while (value > max)
  {
    LOG_INFO(value << " -> " << value - span);
    value -= span;
  }
  while (value < min)
  {
    LOG_INFO(value << " -> " << value + span);
    value += span;
  }
  return value;
};

const TiXmlElement*
XMLIOBase
::child(const TiXmlElement* elem, const std::string& name)
{
  const TiXmlElement* const child_elem = elem->FirstChildElement(name.c_str());
  if (!child_elem) throw MissingChildException(elem, name);
  return child_elem;
}

const TiXmlElement*
XMLIOBase
::nextChild(const TiXmlElement* elem,
            const std::string& name,
            const TiXmlElement* previous)
{
  const TiXmlNode* child_node = previous;
  while ((child_node = elem->IterateChildren(name, child_node)))
  {
    const TiXmlElement* child_elem = child_node->ToElement();
    if (child_elem) return child_elem;
  }
  return 0;
}

void
XMLIOBase
::fromXML(const TiXmlElement* elem,
          const std::string& name,
          std::string& value,
          bool required)
{
  try
  {
    const TiXmlElement* const child_elem = XMLIOBase::child(elem, name);
    const char* const text = child_elem->GetText();
    if (text)
      value = text;
    else
      value.clear();
  }
  catch (const Exception& e)
  {
    if (required)
    {
      XMLIOBase::log(e);
      throw;
    }
    value.clear();
  }
}

template <typename RValue, typename LValue>
RValue
XMLIOBase
::fromXML(const TiXmlElement* elem,
          const std::string& name,
          bool required,
          LValue unset)
{
  try
  {
    const TiXmlElement* const child_elem = XMLIOBase::child(elem, name);
    const std::string text = child_elem->GetText();
    try
    {
      return boost::lexical_cast<RValue>(text);
    }
    catch (const boost::bad_lexical_cast&)
    {
      ++this->_error_count;
      throw ParseException(child_elem, elem);
    }
  }
  catch (const Exception& e)
  {
    if (required)
    {
      XMLIOBase::log(e);
      throw;
    }
    return unset;
  }
}

STANAG_4676_IMPLEMENT_POD_FROMXML_HELPER(int)
STANAG_4676_IMPLEMENT_POD_FROMXML_HELPER(double)
STANAG_4676_IMPLEMENT_OBJECT_FROMXML_HELPER(boost::uuids::uuid)

void
XMLIOBase
::fromXML(const TiXmlElement* elem,
          const std::string& name,
          boost::posix_time::ptime& value,
          bool required)
{
  try
  {
    const TiXmlElement* const child_elem = XMLIOBase::child(elem, name);
    const std::string text = child_elem->GetText();

    // Must validate that string matches expected format first, or boost will
    // just hang indefinitely(!)
    static char const* const format_re =
      "\\d{1,4}-\\d{1,2}-\\d{1,2}"
      "T"
      "\\d{1,2}:\\d{1,2}:\\d{1,2}(\\.\\d{1,6})?"
      "Z";
    if (text.empty() || !boost::regex_match(text, boost::regex(format_re)))
    {
      ++this->_error_count;
      throw ParseException(child_elem, elem);
    }

    // Okay, should be safe to parse the time
    typedef boost::posix_time::ptime ptime;
    value = boost::date_time::parse_delimited_time<ptime>(text, 'T');
  }
  catch (const Exception& e)
  {
    if (required)
    {
      XMLIOBase::log(e);
      throw;
    }
    value = boost::posix_time::ptime();
  }
}

template <typename Enum, Enum Unset>
Enum
XMLIOBase
::fromXML(const TiXmlElement* elem,
          const std::string& name,
          bool required)
{
  try
  {
    const TiXmlElement* const child_elem = child(elem, name);
    const Enum value = fromString<Enum>(child_elem->GetText());

    if (value == Unset)
    {
      ++this->_error_count;
      throw ParseException(child_elem, elem);
    }

    return value;
  }
  catch (const Exception& e)
  {
    if (required)
    {
      XMLIOBase::log(e);
      throw;
    }
    return Unset;
  }
}

STANAG_4676_IMPLEMENT_ENUM_XML_HELPER(ClassificationLevel)
STANAG_4676_IMPLEMENT_ENUM_XML_HELPER(TrackStatus)
STANAG_4676_IMPLEMENT_ENUM_XML_HELPER(ExerciseIndicator)
STANAG_4676_IMPLEMENT_ENUM_XML_HELPER(SimulationIndicator)
STANAG_4676_IMPLEMENT_ENUM_XML_HELPER(TrackPointType)
STANAG_4676_IMPLEMENT_ENUM_XML_HELPER(ModalityType)

void
XMLIOBase
::fromXML(const TiXmlElement* elem,
          const std::string& name,
          std::vector<std::string>& value)
{
  value.clear();

  const TiXmlElement* child_elem = 0;
  while ((child_elem = XMLIOBase::nextChild(elem, name, child_elem)))
  {
    value.push_back(child_elem->GetText());
  }
}


void
XMLIOBase
::typeError(int* error_count, const TiXmlElement* elem)
{
  if (error_count) ++(*error_count);
  XMLIOBase::log(MissingTypeException(elem));
}

void
XMLIOBase
::typeError(int* error_count, const TiXmlElement* elem, const std::string& type)
{
  if (error_count) ++(*error_count);
  XMLIOBase::log(UnsupportedTypeException(elem, type));
}

void
XMLIOBase
::log(const Exception& e)
{
  LOG_WARN(e.what());
}

} // End STANAG_4676 namespace

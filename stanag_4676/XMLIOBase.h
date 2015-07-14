/*ckwg +5
 * Copyright 2013-2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_XMLIOBASE_H_
#define VIDTK_LIBRARY_STANAG_4676_XMLIOBASE_H_

#include <string>
#include <vector>
#include <vbl/vbl_ref_count.h>
#include <vbl/vbl_smart_ptr.h>
#include <boost/date_time.hpp>
#include <boost/uuid/uuid.hpp>
#include <tinyxml.h>
#include <stanag_4676/Enums.h>
#include <stanag_4676/Exceptions.h>
#include <stanag_4676/Macros.h>

namespace STANAG_4676
{

/// \brief Base class of all the XML reader / writer types
class XMLIOBase : public vbl_ref_count
{
public:
  typedef vbl_smart_ptr<XMLIOBase> sptr;

  virtual ~XMLIOBase(void) = 0;

  virtual TiXmlElement* toXML(const std::string& name) const = 0;

protected:
  XMLIOBase(void);

  // Helper functions for XML conversions
  STANAG_4676_DECLARE_POD_XML_HELPER(int)
  STANAG_4676_DECLARE_POD_XML_HELPER(double)
  STANAG_4676_DECLARE_OBJECT_XML_HELPER(std::string)
  STANAG_4676_DECLARE_OBJECT_XML_HELPER(boost::uuids::uuid)
  STANAG_4676_DECLARE_OBJECT_XML_HELPER(boost::posix_time::ptime)
  STANAG_4676_DECLARE_POD_XML_HELPER(ClassificationLevel)
  STANAG_4676_DECLARE_POD_XML_HELPER(TrackStatus)
  STANAG_4676_DECLARE_POD_XML_HELPER(ExerciseIndicator)
  STANAG_4676_DECLARE_POD_XML_HELPER(SimulationIndicator)
  STANAG_4676_DECLARE_POD_XML_HELPER(TrackPointType)
  STANAG_4676_DECLARE_POD_XML_HELPER(ModalityType)

  void fromXML(const TiXmlElement* elem, const std::string& name,
               std::vector<std::string>& value);

  template <typename T>
  void fromXML(const TiXmlElement* elem, const std::string& name,
               typename T::sptr& ptr, bool required = true)
  {
    typedef typename T::sptr Tsptr;

    const TiXmlElement* const child_elem = elem->FirstChildElement(name);
    if (child_elem)
    {
      if ( (ptr = T::fromXML(child_elem, &this->_error_count)) )
      {
        return;
      }

      if (required)
      {
        throw ParseException(child_elem, elem);
      }
      else
      {
        ++this->_error_count;
      }
    }

    if (required)
    {
      throw MissingChildException(elem, name);
    }
    else
    {
      ptr = Tsptr();
    }
  }

  static const TiXmlElement* child(const TiXmlElement* elem,
                                   const std::string& name);
  static const TiXmlElement* nextChild(const TiXmlElement* elem,
                                       const std::string& name,
                                       const TiXmlElement* previous);

  // Wrap a value to a given range
  static double wrapToRange(double value, double min, double max, double span);

  // Helper functions to report fromXML errors
  static void typeError(int* error_count, const TiXmlElement* elem);
  static void typeError(int* error_count, const TiXmlElement* elem,
                        const std::string& type);
  static void log(const Exception& e);

  // Non-fatal error count from fromXML()
  int _error_count;

private:
  template <typename RValue, typename LValue>
  RValue fromXML(const TiXmlElement* elem, const std::string& name,
                 bool required, LValue unset);

  template <typename Enum, Enum Unset>
  Enum fromXML(const TiXmlElement* elem, const std::string& name,
               bool required);
};
inline XMLIOBase::~XMLIOBase(void) { }

} // End STANAG_4676 namespace
#endif

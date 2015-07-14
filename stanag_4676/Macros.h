/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_MACROS_H_
#define VIDTK_LIBRARY_STANAG_4676_MACROS_H_

// Downcast helpers

#define STANAG_4676_DECLARE_DOWNCAST(Type) \
  /** \brief Cast to Type. Will return null if not a Type. */ \
  virtual Type* to##Type(void) { return 0; } \
  /** \brief Cast to Type. Will return null if not a Type. */ \
  virtual const Type* to##Type(void) const { return 0; }

#define STANAG_4676_IMPLEMENT_DOWNCAST(Type) \
  /** \brief Cast to Type. */ \
  virtual Type* to##Type(void) { return this; } \
  /** \brief Cast to Type. */ \
  virtual const Type* to##Type(void) const { return this; }

// Declare class read/write attributes

#define STANAG_4676_DECLARE_ATTRIBUTE(RType, LType, Name) \
  RType get##Name(void) const; \
  /** \copydoc get##Name(void) */ \
  void set##Name(LType);

#define STANAG_4676_DECLARE_POD_ATTRIBUTE(Type, Name) \
  STANAG_4676_DECLARE_ATTRIBUTE(Type, Type, Name)

#define STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(Type, Name) \
  STANAG_4676_DECLARE_ATTRIBUTE(Type, Type const&, Name)

// Implement class read/write attributes

#define STANAG_4676_IMPLEMENT_ATTRIBUTE(Class, RType, LType, Name, Member) \
  RType Class::get##Name(void) const \
  { \
    return this->Member; \
  } \
  void Class::set##Name(LType value) \
  { \
    this->Member = value; \
  }

#define STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(Class, Type, Name, Member) \
  STANAG_4676_IMPLEMENT_ATTRIBUTE(Class, Type, Type, Name, Member)

#define STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(Class, Type, Name, Member) \
  STANAG_4676_IMPLEMENT_ATTRIBUTE(Class, Type, Type const&, Name, Member)

// Declare XMLIOBase toXML / fromXML helper methods

#define STANAG_4676_DECLARE_XML_HELPER(toType, fromType) \
  static TiXmlElement* toXML(const std::string& name, toType value); \
  void fromXML(const TiXmlElement* elem, const std::string& name, \
               fromType value, bool required = true);

#define STANAG_4676_DECLARE_POD_XML_HELPER(Type) \
  STANAG_4676_DECLARE_XML_HELPER(Type, Type&)

#define STANAG_4676_DECLARE_OBJECT_XML_HELPER(Type) \
  STANAG_4676_DECLARE_XML_HELPER(Type const&, Type&)

// Implement fromXML helper method for a 'simple' (can use lexical_cast) type

#define STANAG_4676_IMPLEMENT_FROMXML_HELPER(RType, LType, unset) \
  void \
  XMLIOBase \
  ::fromXML(const TiXmlElement* elem, \
            const std::string& name, \
            RType& value, \
            bool required) \
  { \
    value = XMLIOBase::fromXML<RType, LType>(elem, name, required, unset); \
  }

#define STANAG_4676_IMPLEMENT_POD_FROMXML_HELPER(Type) \
  STANAG_4676_IMPLEMENT_FROMXML_HELPER( \
    Type, Type, std::numeric_limits<Type>::min())

#define STANAG_4676_IMPLEMENT_OBJECT_FROMXML_HELPER(Type) \
  STANAG_4676_IMPLEMENT_FROMXML_HELPER(Type, Type const&, Type())

// Implement toXML / fromXML helper methods for an enum type

#define STANAG_4676_IMPLEMENT_ENUM_XML_HELPER(Enum) \
  TiXmlElement* \
  XMLIOBase \
  ::toXML(const std::string& name, Enum value) \
  { \
    return toXML(name, toString(value)); \
  } \
  \
  void \
  XMLIOBase \
  ::fromXML(const TiXmlElement* elem, \
            const std::string& name, \
            Enum& value, \
            bool required) \
  { \
    value = XMLIOBase::fromXML<Enum, Enum##_UNSET>(elem, name, required); \
  }

// Implement fromXML return of a derived type

#define STANAG_4676_FROMXML_DOWNCAST(elem, type, Type) \
  if (*type == #Type) \
  { \
    return Type::fromXML(elem, error_count); \
  }

// Implement static fromXML method

#define STANAG_4676_FROMXML(Type) \
  Type::sptr \
  Type \
  ::fromXML(const TiXmlElement* elem, int* error_count) \
  { \
    try \
    { \
      const sptr ptr(new Type(elem)); \
      if (error_count) *error_count += ptr->_error_count; \
      return ptr; \
    } \
    catch (const Exception& e) \
    { \
      XMLIOBase::log(e); \
    } \
    \
    if (error_count) ++(*error_count); \
    return sptr(); \
  }

#endif

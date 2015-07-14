/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_IDDATA_H_
#define VIDTK_LIBRARY_STANAG_4676_IDDATA_H_

#include <stanag_4676/XMLIOBase.h>

namespace STANAG_4676
{

/// \brief Provides identification information for a given STANAG 4676 capable
///        system.
///
/// The combination of stationID and nationality xs:elements provides unique
/// identification of any given STANAG 4676 capable system.
class IDdata : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<IDdata> sptr;

  IDdata(const std::string& id, const std::string& nationality);

  virtual ~IDdata(void) { }

  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::string, Id)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::string, Nationality)

  TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  IDdata(const TiXmlElement* elem);

private:
  std::string _id;
  std::string _nationality;
};

} // End STANAG_4676 namespace

#endif

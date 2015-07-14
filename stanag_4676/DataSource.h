/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_DATASOURCE_H_
#define VIDTK_LIBRARY_STANAG_4676_DATASOURCE_H_

#include <string>
#include <boost/date_time.hpp>
#include <stanag_4676/XMLIOBase.h>

namespace STANAG_4676
{

/// \brief Provides information about the source of data used to create the
///        SPADE track message.
class DataSource : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<DataSource> sptr;

  DataSource(const std::string& name, const boost::posix_time::ptime& t);

  virtual ~DataSource(void) { }

  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::string, Name)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(boost::posix_time::ptime, Time)

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  DataSource(const TiXmlElement* elem);

private:
  std::string _name;
  boost::posix_time::ptime _time;
};

} // End STANAG_4676 namespace

#endif

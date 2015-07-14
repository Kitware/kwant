/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_TRACKSOURCE_H_
#define VIDTK_LIBRARY_STANAG_4676_TRACKSOURCE_H_

#include <stanag_4676/XMLIOBase.h>

namespace STANAG_4676
{

/// \brief Provides identification information about the specific tracker used
///        to generate the tracks within the SPADE track message.
class TrackSource : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<TrackSource> sptr;

  TrackSource(const std::string& name, const std::string& desc);

  virtual ~TrackSource(void) { }

  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::string, Name)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::string, Description)

  TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  TrackSource(const TiXmlElement* elem);

private:
  std::string _name;
  std::string _description;
};

} // End STANAG_4676 namespace

#endif

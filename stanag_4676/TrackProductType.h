/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_TRACKPRODUCTTYPE_H_
#define VIDTK_LIBRARY_STANAG_4676_TRACKPRODUCTTYPE_H_

#include <stanag_4676/XMLIOBase.h>
#include <stanag_4676/TrackMessage.h>

namespace STANAG_4676
{

/// \brief This type represents the composite of track messages: 4676
class TrackProductType : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<TrackProductType> sptr;

  TrackProductType(TrackMessage::sptr msg);

  virtual ~TrackProductType(void) { }

  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(TrackMessage::sptr, Message)

  virtual TiXmlElement* toXML(const std::string& name) const;
  TiXmlElement* toXML(void) const { return this->toXML("trackProduct"); }

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  TrackProductType(const TiXmlElement* elem);

private:
  TrackMessage::sptr _message;
};

} // End STANAG_4676 namespace

#endif

/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_TRACK_H_
#define VIDTK_LIBRARY_STANAG_4676_TRACK_H_

#include <stanag_4676/XMLIOBase.h>
#include <stanag_4676/Enums.h>
#include <stanag_4676/Security.h>
#include <stanag_4676/TrackItem.h>

#include <boost/uuid/uuid.hpp>

namespace STANAG_4676
{

/// \brief Provides parameters related to a track.
///
/// Top-level information about the track is expressed in the Track class
/// itself. Track points and other information are expressed in track items
/// (see class TrackItem).
class Track : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<Track> sptr;

  Track(const boost::uuids::uuid& uuid, const std::string& number,
        Security::sptr security, const ExerciseIndicator& e,
        const SimulationIndicator& s);

  virtual ~Track(void) { }

  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(boost::uuids::uuid, UUID)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::string, Number)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(TrackStatus, Status)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(Security::sptr, Security)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::string, Comment)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(ExerciseIndicator, ExerciseIndicator)
  STANAG_4676_DECLARE_POD_ATTRIBUTE(SimulationIndicator, SimulationIndicator)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::vector<TrackItem::sptr>, Items)

  void addItem(TrackItem::sptr item);

  virtual TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  Track(const TiXmlElement* elem);

private:
  boost::uuids::uuid _uuid;
  std::string _number;
  TrackStatus _status;
  Security::sptr _security;
  std::string _comment;
  ExerciseIndicator _exerciseIndicator;
  SimulationIndicator _simulationIndicator;
  std::vector<TrackItem::sptr> _items;
};

} // End STANAG_4676 namespace

#endif

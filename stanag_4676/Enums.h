/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_ENUMS_H_
#define VIDTK_LIBRARY_STANAG_4676_ENUMS_H_

#include <string>
#include <tinyxml.h>

namespace STANAG_4676
{

template <typename T> T fromString(const std::string& name);

/// \brief Provides security classification levels
enum ClassificationLevel
{
  TOP_SECRET   = 0x0000,
  SECRET       = 0x0001,
  CONFIDENTIAL = 0x0002,
  RESTRICTED   = 0x0003,
  UNCLASSIFIED = 0x0004,
  ClassificationLevel_UNSET = 0xFFFF
};
const std::string& toString(ClassificationLevel l);
template <> ClassificationLevel fromString(const std::string& name);

/// \brief Provides the status of a track (i.e. initiating, maintaining,
///        dropping, terminated).
enum TrackStatus
{
  INITIATING  = 0x0000,
  MAINTAINING = 0x0001,
  DROPPING    = 0x0002,
  TERMINATED  = 0x0003,
  TrackStatus_UNSET = 0xFFFF
};
const std::string& toString(TrackStatus stat);
template <> TrackStatus fromString(const std::string& name);

/// \brief Provides an indication of whether the information pertains to
///        operational data, exercise data, or test data.
enum ExerciseIndicator
{
  OPERATIONAL = 0x0000,
  EXERCISE    = 0x0001,
  TEST        = 0x0002,
  ExerciseIndicator_UNSET = 0xFFFF
};
const std::string& toString(ExerciseIndicator ex);
template <> ExerciseIndicator fromString(const std::string& name);

/// \brief Provides an indication of whether reported information is real,
///        simulated, or synthesized.
enum SimulationIndicator
{
  REAL        = 0x0000,
  SIMULATED   = 0x0001,
  SYNTHESIZED = 0x0002,
  SimulationIndicator_UNSET = 0xFFFF
};
const std::string& toString(SimulationIndicator sim);
template <> SimulationIndicator fromString(const std::string& name);

/// \brief Provides information about the type of track point (i.e. measured,
///        estimated, predicted).
enum TrackPointType
{
  /// \brief Indicates a measured track point.
  ///
  /// A detection marked as a track point, with no additional adjustments,
  /// automatic/machine filtering, or estimation processing (i.e."raw"
  /// detection information, or input to the tracker).
  MEASURED            = 0x0000,

  /// \brief Indicates a manual, estimated track point.
  ///
  /// Position is approximated by an operator/analyst, based on one or more
  /// measurements and his/her analytical judgment (example: "snap to road").
  MANUAL_ESTIMATED    = 0x0001,

  /// \brief Indicates a manual, predicted track point.
  ///
  /// A point provided by operator/analyst that is based on prior track
  /// history, but is not associated with a direct measurement.
  MANUAL_PREDICTED    = 0x0002,

  /// \brief Indicates an automatic, estimated track point.
  ///
  /// A point provided by automatic tracker, based on one or more measurements
  /// and automatic adjustments (example: "snap to road").
  AUTOMATIC_ESTIMATED = 0x0003,

  /// \brief Indicates an automatic, predicted track point.
  ///
  /// A point provided by automatic tracker, based on prior track history, but
  /// is not associated with a direct measurement.
  AUTOMATIC_PREDICTED = 0x0004,

  TrackPointType_UNSET = 0xFFFF
};
const std::string& toString(TrackPointType t);
template <> TrackPointType fromString(const std::string& name);

/// \brief Provides the type and source of information from which information
///        was computed or derived.
enum ModalityType
{
  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from a radar Doppler source.
  DOPPLER_SIGNATURE  = 0x0000,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from a radar High Range Resolution source.
  HRR_SIGNATURE      = 0x0001,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from a Still or MOTION IMAGERY source.
  IMAGE_SIGNATURE    = 0x0002,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from a Human Intelligence source.
  HUMINT             = 0x0003,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from a Measurement and Signal Intelligence source.
  MASINT             = 0x0004,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from a Electronics Intelligence source.
  ELINT              = 0x0005,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from a Communications Intelligence Externals source.
  COMINT_EXTERNALS   = 0x0006,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from a Communications Intelligence Internals source.
  COMINT_INTERNALS   = 0x0007,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from a Open Source Intelligence source.
  OSINT              = 0x0008,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from a Biometrics source.
  BIOMETRICS         = 0x0009,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from an Automated Identification System source.
  AIS                = 0x0010,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from a Blue Force Tracking source.
  BFT                = 0x0011,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from a combination of two or more sources.
  MIXED              = 0x0012,

  /// \brief Indicates that the information, estimate, or determination is
  ///        derived from other types of sources, such as Link 16.
  OTHER              = 0x0013,

  ModalityType_UNSET = 0xFFFF
};
const std::string& toString(ModalityType t);
template <> ModalityType fromString(const std::string& name);

} // End STANAG_4676 namespace

#endif

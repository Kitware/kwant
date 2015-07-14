/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "Enums.h"

#include <stdexcept>

namespace // anon
{

#if (__GNUC__ >= 4) || (__cplusplus >= 201103L && !defined(_MSC_VER))

#define GUARD(Enum)
#define GET_MAP_PARAMS size_t (*getMap)(std::string const*&)
#define GET_MAP(func, map) (*getMap)(map)
#define ENUM_PARAMS(Enum) &get##Enum##Map

#elif defined(_MSC_VER)

#include <Windows.h>

typedef LONG volatile* guard_t;

static inline size_t
lockedGetMap(size_t (*getMap)(std::string const*&),
             std::string const*& map,
             guard_t guard)
{
  if (*guard != 2)
  {
    if (InterlockedCompareExchange(guard, 1, 0) == 0)
    {
      size_t const result = (*getMap)(map);
      InterlockedExchange(guard, 2);
      return result;
    }

    while (InterlockedCompareExchange(guard, 2, 2) != 2) { /* empty*/ }
  }
  return (*getMap)(map);
}

#define GUARD(Enum) static LONG volatile guard##Enum = 0;
#define GET_MAP_PARAMS size_t (*getMap)(std::string const*&), guard_t guard
#define GET_MAP(func, map) lockedGetMap(func, map, guard)
#define ENUM_PARAMS(Enum) &get##Enum##Map, &guard##Enum

#else

#error No support for thread-safe static initialization on this platform!

#endif

#define TO_STRING(Enum) \
  std::string const& toString(Enum i) \
  { return ::toString(i, ENUM_PARAMS(Enum), #Enum); }

#define FROM_STRING(Enum) \
  template <> Enum fromString(std::string const& name) \
  { return ::fromString<Enum>(name, Enum##_UNSET, ENUM_PARAMS(Enum)); }

#define TO_FROM_STRING(Enum) \
  TO_STRING(Enum) \
  FROM_STRING(Enum)

#define DECLARE_MAP(Enum, ...) \
  GUARD(Enum) \
  size_t get##Enum##Map(std::string const*& map) \
  { \
    static const std::string _map[] = { __VA_ARGS__ }; \
    map = _map; \
    return (sizeof(_map) / sizeof(std::string&)); \
  }

template <typename T>
std::string const&
toString(T i, GET_MAP_PARAMS, char const* name)
{
  std::string const* map;
  size_t const count = GET_MAP(getMap, map);

  size_t const x = static_cast<size_t>(i);
  if (x >= count)
  {
    std::ostringstream oss;
    oss << "STANAG_4676::toString(" << name << "): index out of bounds";
    throw std::range_error(oss.str());
  }
  return map[x];
}

template <typename T>
T
fromString(std::string const& text, T invalidValue, GET_MAP_PARAMS)
{
  std::string const* map;
  size_t const count = GET_MAP(getMap, map);

  for (size_t i = 0; i < count; ++i)
  {
    if (text == map[i])
      return static_cast<T>(i);
  }
  return invalidValue;
}

DECLARE_MAP(ClassificationLevel,
  "TOP SECRET",
  "SECRET",
  "CONFIDENTIAL",
  "RESTRICTED",
  "UNCLASSIFIED")

DECLARE_MAP(TrackStatus,
  "INITIATING",
  "MAINTAINING",
  "DROPPING",
  "TERMINATED")

DECLARE_MAP(ExerciseIndicator,
  "OPERATIONAL",
  "EXERCISE",
  "TEST")

DECLARE_MAP(SimulationIndicator,
  "REAL",
  "SIMULATED",
  "SYNTHESIZED")

DECLARE_MAP(TrackPointType,
  "MEASURED",
  "MANUAL ESTIMATED",
  "MANUAL PREDICTED",
  "AUTOMATIC ESTIMATED",
  "AUTOMATIC PREDICTED")

DECLARE_MAP(ModalityType,
  "DOPPLER SIGNATURE",
  "HRR SIGNATURE",
  "IMAGE SIGNATURE",
  "HUMINT",
  "MASINT",
  "ELINT",
  "COMINT EXTERNALS",
  "COMINT INTERNALS",
  "OSINT",
  "BIOMETRICS",
  "AIS",
  "BFT",
  "MIXED",
  "OTHER")

}

namespace STANAG_4676
{

TO_FROM_STRING(ClassificationLevel)
TO_FROM_STRING(TrackStatus)
TO_FROM_STRING(ExerciseIndicator)
TO_FROM_STRING(SimulationIndicator)
TO_FROM_STRING(TrackPointType)
TO_FROM_STRING(ModalityType)

} // End STANAG_4676 namespace

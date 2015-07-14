/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef vidtk_event_types_h_
#define vidtk_event_types_h_

/**
  \file
  \brief
  Method and field definitions for event type.
*/

namespace vidtk
{
///Parent class to all event types
class event_types
{
  public:
  ///Enumerated types of the defined system events.
  enum enum_types
  {
    //generate enum and string names from same source to ensure match
    #define EVENT_NAME(name) name##_EVENT
    #include "event_names.h"
    #undef EVENT_NAME
  };

  ///C-strings of system event names.
  static const char * event_names[];

  /// Array of enumerated types of vehicle events.
  static const enum_types vehicle_events[];

  /// Number of defined vehicle events.
  static const unsigned int vehicle_events_size;

  static const enum_types human_events[];
  static const unsigned int human_events_size;

  /// Array of system defined events.
  static const enum_types events[];

  /// Number of defined events.
  static const unsigned int events_size = MAX_EVENT;

};

}// end namespace vidtk

#endif  //#ifndef event_types_h_

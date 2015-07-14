/*ckwg +5
 * Copyright 2010-2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "timestamp.h"

#include <vul/vul_sprintf.h>
#include <vcl_ctime.h>
#include <vcl_cstring.h>
#include <boost/foreach.hpp>

#include <logger/logger.h>
#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __vidtk_logger_auto_timestamp_cxx__
VIDTK_LOGGER("timestamp_cxx");




namespace vidtk
{


bool
timestamp
::operator<( timestamp const& other ) const
{
  if( this->has_time() && other.has_time() )
  {
    return this->time() < other.time();
  }
  else if( this->has_time() || other.has_time() )
  {
    LOG_WARN( "comparing timestamps that don't both have time values" );
    return false;
  }
  else if( this->has_frame_number() && other.has_frame_number() )
  {
    return this->frame_number() < other.frame_number();
  }
  else
  {
    LOG_WARN( "comparing timestamps that don't both have time values or frame number values, or are empty" );
    return false;
  }
}

bool
timestamp
::operator>( timestamp const& other ) const
{
  return !(*this <= other );
}


bool
timestamp
::operator==( timestamp const& other ) const
{
  if( this->has_time() && other.has_time() )
  {
    return this->time() == other.time();
  }
  else if( this->has_time() || other.has_time() )
  {
    LOG_WARN( "comparing timestamps that don't both have time values" );
    return false;
  }
  else if( this->has_frame_number() && other.has_frame_number() )
  {
    return this->frame_number() == other.frame_number();
  }
  else
  {
    LOG_WARN( "comparing timestamps that don't both have time values or frame number values, or are empty" );
    return false;
  }
}


bool
timestamp
::operator<=( timestamp const& other ) const
{
  return (*this < other ) || (*this == other);
}

bool
timestamp
::operator>=( timestamp const& other ) const
{
  return (*this > other ) || (*this == other);
}


void
timestamp
::shift_forward( timestamp const& other)
{
  if( this->has_time() && other.has_time() )
  {
    time_ += other.time();
  }
  if( this->has_frame_number() && other.has_frame_number() )
  {
    frame_num_ += other.frame_number();
  }
}


void
timestamp
::shift_backward( timestamp const& other)
{
  if( this->has_time() && other.has_time() )
  {
    time_ -= other.time();
  }
  if( this->has_frame_number() && other.has_frame_number() )
  {
    frame_num_ -= other.frame_number();
  }
}


// ----------------------------------------------------------------
/** Format timestamp on a stream.
 *
 *
 */
vcl_ostream & operator<< (vcl_ostream& str, const vidtk::timestamp& obj)
{
  vcl_string c_tim("");
  time_t tt = static_cast< time_t >(obj.time() * 1e-6);
  char buffer[128];


  str << "ts(f: ";

  if (obj.has_frame_number())
  {
    str << obj.frame_number();
  }
  else
  {
    str << "<inv>";
  }

  str << ", t: ";

  if (obj.has_time())
  {
    char* p = vcl_ctime(&tt); // this may return null if tt is out of range,
    if (p)
    {
      c_tim = " (";
      buffer[0] = 0;
      vcl_strncpy (buffer, p, sizeof buffer);
      buffer[vcl_strlen(buffer)-1] = 0; // remove NL

      c_tim = c_tim + buffer;
      c_tim = c_tim + ")";

      str << vul_sprintf("%.6f", obj.time() * 1e-6 )
          << c_tim;
    }
    else
    {
      str << " (tt " << tt << " out of bounds?)";
    }
  }
  else
  {
    str << "<inv>";
  }

  str << ")";

  return (str);
}


// ----------------------------------------------------------------
/** Print timestamp vector.
 *
 * This operator prints a vector of timestamps.
 *
 * @param[in] str - stream to format on
 * @param[in] obj - vector to format.
 *
 * @return The original stream is returned.
 */
vcl_ostream & operator<< (vcl_ostream& str, const vidtk::timestamp::vector_t& obj)
{
  int idx(0);

  str << "Timestamp vector size: " << obj.size() << "\n";
  BOOST_FOREACH (vidtk::timestamp const& ts, obj)
  {
    str << "vector[" << idx << "] :" << ts << "\n";
    idx++;
  }

  return (str);
}


} // end namespace vidtk

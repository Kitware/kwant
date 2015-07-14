/*ckwg +5
 * Copyright 2010-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef vidtk_timestamp_h_
#define vidtk_timestamp_h_

#include <vxl_config.h>
#include <ostream>
#include <vector>

/**\file
   \brief
   Method and field definition for timestamps
*/

namespace vidtk
{

/// \brief Represents a world time in microseconds (10^-6 seconds).
///
/// A timestamp contains two pieces of information: a time and a frame
/// number.  Generally, one should use the time value over the frame
/// number if both are available.
class timestamp
{
public:
  typedef std::vector < timestamp > vector_t;

  /// \brief Default constructor.
  timestamp()
    : time_( -1e300 ),
      frame_num_( static_cast<unsigned int>(-1) )
  {
  }

  /// \brief Constructor : pass a time.
  ///
  /// The constructor is explicit to make sure that the user did
  /// intend to create a timestamp that does not have a frame number.
  explicit timestamp( double t )
    : time_( t ),
      frame_num_( static_cast<unsigned int>(-1) )
  {
  }

  /// \brief Constructor:
  ///
  /// pass a time and a frame number.
  timestamp( double t, unsigned f )
    : time_( t ),
      frame_num_( f )
  {
  }
  ///Returns true if has time; otherwise false.
  bool has_time() const
  {
    // these magic numbers perhaps should be replaced by explicit bools.
    return time_ != -1e300;
  }

  ///Return the time.
  double time() const
  {
    return time_;
  }
  ///Set the time.
  void set_time( double t )
  {
    time_ = t;
  }
  ///Returns true if the frame number is found; otherwise false.
  bool has_frame_number() const
  {
    // these magic numbers perhaps should be replaced by explicit bools.
    return frame_num_ != static_cast<unsigned int>(-1);
  }
  ///Return the frame number.
  unsigned frame_number() const
  {
    return frame_num_;
  }

  ///Set the frame number.
  void set_frame_number( unsigned f )
  {
    frame_num_ = f;
  }


  /** Is timestamp valid. This method determines if the timestamp is
   * valid if it has either a valid time, or a valid frame number, or
   * both.
   */
  bool is_valid() const
  {
    return (has_time() || has_frame_number());
  }


  /// \brief Return the time or frame number, depending on
  /// availability.
  ///
  /// If a time value is available, this will be a time value.  If
  /// not, it will be the frame number.
  ///
  /// The return value is const to avoid mistakes like
  /// \code
  ///   timestamp t;
  ///   t += 1.0;
  /// \endcode
//   operator double const() const
//   {
//     return has_time() ? time_ : double( frame_num_ );
//   }

  /// \brief Return the time or frame number, depending on
  /// availability.
  ///
  /// If a time value is available, this will be a time value in seconds.  If
  /// not, it will be the frame number.
  double time_in_secs() const
  {
    return has_time() ? time_/1e6 : double( frame_num_ );
  }


  /// \brief Return the time or frame number difference, depending on
  /// availability.
  ///
  /// The return value is conceptually <tt>*this - \a subtrahend</tt>
  /// expressed in seconds or in frames.
  ///
  /// If a time value is available in both numbers, the result is the
  /// difference expressed in seconds.  Otherwise, if the frame number
  /// if available in both, the result is the difference in frame
  /// numbers.  Finally, if neither of these hold, the result is
  /// undefined.
  double diff_in_secs( timestamp const& subtrahend ) const
  {
    if( has_time() && subtrahend.has_time() )
    {
      return (time_ - subtrahend.time_) / 1e6;
    }
    else
    {
      return double( frame_num_ ) - double( subtrahend.frame_num_ );
    }
  }

  /// \brief Ordering on timestamps.
  ///
  /// If the timestamps have time values, the order is based on time.
  /// Otherwise, if neither have time values, but have frame numbers,
  /// the order is based on frame number.  Otherwise, the comparison
  /// will always return \c false.
  ///
  /// Note that this means if one has a time value but the other does
  /// not, the result will be \c false.
  bool operator<( timestamp const& other ) const;
  bool operator>( timestamp const& other ) const;

  /// \brief Equality of timestamps.
  ///
  /// If the timestamps have time values, the comparison is based on
  /// time.  If neither have time values, but both have frame numbers,
  /// the comparison is based on frame number.  In all other cases,
  /// the result is \c false.
  bool operator==( timestamp const& other ) const;
  bool operator!=( timestamp const& other ) const { return (! this->operator==(other) ); }

  /// \brief Combined ordering and equality operator
  ///
  /// Logically identical to (*this < other ) || (*this == other)
  bool operator<=( timestamp const& other ) const;

  /// \brief Combined ordering and equality operator
  ///
  /// Logically identical to (*this > other ) || (*this == other)
  bool operator>=( timestamp const& other ) const;

  /// \brief Shift the timestamps.
  ///
  /// Shift the timestamp forward
  void shift_forward( timestamp const& other);

  /// Shift the timestamp backward
  void shift_backward( timestamp const& other);


private:
  double time_;
  unsigned frame_num_;
};


std::ostream & operator<< (std::ostream& str, const vidtk::timestamp& obj);
std::ostream & operator<< (std::ostream& str, const vidtk::timestamp::vector_t& obj);


} // end namespace vidtk


#endif // vidtk_timestamp_h_

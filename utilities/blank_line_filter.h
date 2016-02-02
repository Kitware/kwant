/*ckwg +5
 * Copyright 2012-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef _VIDTK_BLANK_LINE_FILTER_H_
#define _VIDTK_BLANK_LINE_FILTER_H_

#include <string>

#include <boost/iostreams/char_traits.hpp> // EOF, WOULD_BLOCK
#include <boost/iostreams/concepts.hpp>    // multichar_input_filter
#include <boost/iostreams/operations.hpp>  // get

namespace vidtk {



// ----------------------------------------------------------------
/** Blank line filter.
 *
 * This filter absorbs blank lines, including the trailing newline.
 *
 * Add this filter to boost::filtering_streams to remove blank lines
 * from a io stream.
 *
 * Example:
\code
#include <boost/iostreams/filtering_stream.hpp>

std::ifstream reg_file( opt.c_str() );

// Build boost filter to remove comments and blank lines.
// Order is important.
boost::iostreams::filtering_istream in_stream;
in_stream.push (vidtk::blank_line_filter());
in_stream.push (vidtk::shell_comments_filter());
in_stream.push (reg_file);

// read file until eof
std::string line;
while ( std::getline(in_stream, line) )
{
    // process 'line'
} // end while
\endcode
 */
class blank_line_filter
  : public boost::iostreams::input_filter
{
public:
  blank_line_filter()
    : state_(ST_INIT)
  { }



// ----------------------------------------------------------------
/* Skip blank lines.
 *
 * This filter absorbs blank lines.  This could be a simpler
 * implementation if we were to make assumptions about leading
 * whitespace.
 *
 * This method is called from the boost::streams framework and should
 * not be called directly unless you know what you are doing.
 */
  template<typename Source>
  int get(Source& src)
  {
    int c;
    while (true)
    {
      switch (this->state_)
      {

      case ST_INIT:
      {
        if ((c = boost::iostreams::get(src)) == EOF || c == boost::iostreams::WOULD_BLOCK)
        {
          return c;
        }

        // looking for non-whitespace and found EOL,
        // drop line and start again.
        if (c == '\n')
        {
          line_buffer_.clear();
          break; // get next character
        }

        // Add character to buffer
        line_buffer_.append(1, c);

        if (! isspace(c))
        {
          // We have non-whitespace character, flush collected
          // characters downstream and pass remaining characters.
          buffer_ptr_ = line_buffer_.begin();
          state_ = ST_PASS_BUFFER; // switch to passing state
        }
        break;
      }

      case ST_PASS_BUFFER:
      {
        // passing buffer contents down stream
        if (buffer_ptr_ == line_buffer_.end())
        {
          line_buffer_.clear();
          state_ = ST_PASS; // switch to passing state
          break; // get next character
        }

        // return buffered character
        int b = *buffer_ptr_;
        buffer_ptr_++;
        return b;
      }

      // Pass characters until newline is seen.
      // pass newline too.
      case ST_PASS:
      {
        if ((c = boost::iostreams::get(src)) == EOF || c == boost::iostreams::WOULD_BLOCK)
        {
          return c;
        }

        if (c == '\n')
        {
          // EOL resets to buffering state
          state_ = ST_INIT;
        }
        return c;
      }

      } // end switch
    } // end while

    return c;
  }


  template<typename Source>
  void close(Source&) { state_ = ST_INIT; }

private:

  enum { ST_INIT, ST_PASS_BUFFER, ST_PASS} state_;


  std::string line_buffer_;
  // Used for flushing buffered lines
  std::string::iterator buffer_ptr_;

};

} // end namespace

#endif /* _VIDTK_BLANK_LINE_FILTER_H_ */

/*ckwg +5
 * Copyright 2012-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef _VIDTK_SHELL_COMMENTS_FILTER_H_
#define _VIDTK_SHELL_COMMENTS_FILTER_H_

#include <boost/iostreams/char_traits.hpp> // EOF, WOULD_BLOCK
#include <boost/iostreams/concepts.hpp>    // multichar_input_filter
#include <boost/iostreams/operations.hpp>  // get


namespace vidtk
{

// ----------------------------------------------------------------
/** Filter to remove shell style comments.
 *
 * This filter removes all characters between the comment character
 * (default '#') up to, but not including, the \n from the stream.
 *
 * Add this filter to boost::filtering_streams to remove comments from
 * a stream. This can be combined with other filters in the
 * filtering_stream.
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
class shell_comments_filter
  : public boost::iostreams::input_filter
{
public:
  // ----------------------------------------------------------------
  /** Constructor.
   *
   * Create a new shell comment filter. The optional parameter
   * specifies the character to use to start the comment.
   * @param[in] comment_char - character to start comment. Default to '#'.
   */

  explicit shell_comments_filter(char comment_char = '#')
    : skip_(false), comment_char_(comment_char)
  { }


// ----------------------------------------------------------------
/* Filter input stream.
 *
 * This method reads one or more characters and produces only one
 * character.
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
      if ((c = boost::iostreams::get(src)) == EOF || c == boost::iostreams::WOULD_BLOCK)
      {
        break;
      }

      skip_ = (c == comment_char_) ?
        true : c == '\n' ?
        false : skip_;

      if (!skip_)
      {
        break;
      }
    }
    return c;
  }


  template<typename Source>
  void close(Source&) { skip_ = false; }


private:
  bool skip_;

  /// Comment character to use
  char comment_char_;

};

} // end namespace


#endif /* _VIDTK_SHELL_COMMENTS_FILTER_H_ */

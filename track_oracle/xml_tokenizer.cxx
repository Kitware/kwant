/*ckwg +5
 * Copyright 2012-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "xml_tokenizer.h"

#include <cstdio>
#include <ctype.h>


using std::fread;
using std::string;
using std::vector;

namespace // anon
{

struct xml_stream_buffer
{
  xml_stream_buffer( FILE* fptr );
  ~xml_stream_buffer();
  bool get_next_char( char& c );

private:
  FILE* fp;
  size_t index, top, bufsize;
  char* buf;
};

xml_stream_buffer
::xml_stream_buffer( FILE* fptr )
  : fp( fptr ), index(0), top(0), bufsize( 2048 )
{
  buf = new char[bufsize];
}

xml_stream_buffer
::~xml_stream_buffer()
{
  delete [] buf;
}


bool
xml_stream_buffer
::get_next_char( char& c )
{
  // if we're at the end of the buffer, try to top it up
  if (index == top)
  {
    // if fp is null, we're done
    if (fp == NULL) return false;

    index = 0;
    top = fread( buf, sizeof(char), bufsize, fp );
    // if we couldn't get a full buffer, then it's either error or EOF
    // either way, we'll be done as soon as this buffer is empty
    if ( top < bufsize ) fp = NULL;

    // if we're STILL at the end of the buffer, we're done
    if (index == top) return false;
  }

  c = buf[ index++ ];
  return true;
}

} // anon


namespace vidtk
{

vector< string>
xml_tokenizer
::first_n_tokens( const string& fn, size_t n )
{
  FILE* fp = fopen( fn.c_str(), "rb" );
  if (fp == NULL)
  {
    return vector<string>();
  }

  xml_stream_buffer xmlbuf( fp );


  vector< string > tokens;
  string this_token = "";
  // state machine:
  // 0 = init (nothing seen)
  // 1 = in a run of whitespace
  // 2 = in a run of not-whitespace
  // 0->1 or 1->2 transitions signal start of a token
  // 2->1 transitions signal completion of a token
  unsigned state = 0;
  bool in_comment = false;
  do
  {
    char this_char;
    if (! xmlbuf.get_next_char( this_char ))
    {
      // EOF / error / whatever; if outside comments, push current token (if any) and exit
      if (( ! in_comment) && (this_token != "")) tokens.push_back( this_token );
      break;
    }

    if (isspace( this_char )) // new state will be 1
    {
      switch (state)
      {
      case 0:
        state = 1;
        break;
      case 1:
        break;
      case 2:
        // check for entering / leaving comments; only store tokens which are
        // outside comments
        if (! in_comment)
        {
          if (this_token == "<!--")
          {
            in_comment = true;
          }
          else
          {
            tokens.push_back( this_token );
          }
        }
        else
        {
          if ((this_token.size() >= 3)
              && (this_token.substr( this_token.size()-3, 3) == "-->"))
          {
            in_comment = false;
          }
        }
        this_token = "";
        state = 1;
        break;
      }
    }
    else  // new state will be 2
    {
      state = 2;
      // quick-n-dirty approach to breaking on "raw" xml streams
      // which may be '<token1><token2><token3>':

      if ((this_char == '<') && (! in_comment))
      {
        tokens.push_back( this_token );
        this_token = "<";
      }
      else if (this_char == '>')
      {
        // does this end a comment?
        this_token += this_char;
        bool this_ends_comment =
          in_comment &&
          (this_token.size() >= 3) &&
          (this_token.substr( this_token.size()-3, 3) == "-->");
        if (in_comment && this_ends_comment )
        {
          // clear the token, but do not store
          this_token = "";
          in_comment = false;
        }
        else
        {
          // store && clear
          tokens.push_back( this_token );
          this_token = "";
        }
      }
      else
      {
        this_token += this_char;
        if (this_token == "<!--")
        {
          in_comment = true;
        }
      }
    }

    // don't store empty tokens
    if ( ! tokens.empty() )
    {
      if (tokens.back() == "")
      {
        tokens.pop_back();
      }
    }
  }
  while (tokens.size() < n );
  fclose( fp );
  return tokens;

}



} // vidtk

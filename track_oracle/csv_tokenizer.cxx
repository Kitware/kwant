/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "csv_tokenizer.h"

#include <logger/logger.h>

#include <istream>

VIDTK_LOGGER( "csv_tokenizer" );

using std::istream;
using std::string;
using std::stringstream;
using std::vector;

namespace vidtk
{
namespace csv_tokenizer
{

istream&
get_record(istream& is, vector<string>& values)
{
  values.clear();

  string token;
  char c;
  bool in_quote = false;

  is.get(c);
  if (is.eof()) return is; // Handle end of file gracefully

  for (; is; is.get(c))
  {
    // Handle DOS and (old-style) Mac line endings
    if (c == '\r')
    {
      if (is.peek() == '\n') continue; // CR before LF is ignored
      c = '\n'; // CR followed by anything else is treated as LF
    }
    else if (c == '"')
    {
      // Value starting with quote character denotes a quoted value
      if (token.empty())
      {
        in_quote = true;
        continue;
      }
      // Otherwise, quotes have special meaning only in quoted value
      else if (in_quote)
      {
        if (is.get(c))
        {
          // Doubled quote is emitted as-is; anything else ends quoting
          in_quote = (c == '"');
        }
        else
        {
          // Stream is now at EOF or in bad state; former is treated as end of
          // record; latter is left to caller to check and report an error
          break;
        }
      }
    }

    if (!in_quote)
    {
      if (c == '\n') break; // End of record
      if (c == ';' || c == ',')
      {
        values.push_back(token);
        token.clear();
        continue;
      }
    }

    token.push_back(c);
  }

  values.push_back(token);
  return is;
}

vector<string>
parse(string const& s)
{
  vector<string> result;

  stringstream ss(s);
  if (!get_record(ss, result) && !ss.eof())
    LOG_WARN("CSV parsing failed for input string '" << s << "'");

  return result;
}

} // csv_tokenizer
} // vidtk

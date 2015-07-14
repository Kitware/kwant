/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "split.h"

#include <boost/algorithm/string.hpp>

using std::vector;
using std::string;

namespace // anon
{

void push_token(vector< string >& out,
                string token,
                int flags)
{
  if (flags & vidtk::SPLIT_TRIM_WHITESPACE)
  {
    boost::algorithm::trim(token);
  }
  if (flags & vidtk::SPLIT_SKIP_EMPTY)
  {
    if (token.empty()) return;
  }
  out.push_back(token);
}

}

namespace vidtk
{

vector< string > split(string const& s,
                       char const* seps,
                       int flags)
{
  vector< string > result;

  string::size_type si = 0, ei;
  while ((ei = s.find_first_of(seps, si)) != string::npos)
  {
    push_token(result, s.substr(si, ei - si), flags);
    si = ei + 1;
  }
  push_token(result, s.substr(si), flags);

  return result;
}

} // vidtk

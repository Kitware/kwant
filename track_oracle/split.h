/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

///
/// A quick-n-dirty CSV tokenizer.
///

#ifndef INCL_SPLIT_H
#define INCL_SPLIT_H

#include <string>
#include <vector>

namespace vidtk
{

enum SPLIT_OPTION
{
  SPLIT_SKIP_EMPTY = 0x1,
  SPLIT_TRIM_WHITESPACE = 0x2,
};

std::vector<std::string> split(std::string const& s, char const* seps, int flags);

} // vidtk

#endif

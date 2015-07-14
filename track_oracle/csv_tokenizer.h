/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

///
/// A quick-n-dirty CSV tokenizer.
///

#ifndef INCL_CSV_TOKENIZER_H
#define INCL_CSV_TOKENIZER_H

#include <iosfwd>
#include <string>
#include <vector>

namespace vidtk
{

namespace csv_tokenizer
{
  std::istream& get_record(std::istream& is, std::vector<std::string>& out);
  std::vector<std::string> parse(std::string const& s);
};

} // vidtk

#endif

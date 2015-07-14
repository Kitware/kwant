/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

///
/// A wee class which returns the first N tokens from a file,
/// skipping XML comments.  May return fewer than N if the file
/// is shorter than N tokens.
///

#ifndef INCL_XML_TOKENIZER_H
#define INCL_XML_TOKENIZER_H

#include <string>
#include <vector>

namespace vidtk
{

struct xml_tokenizer
{
  static std::vector< std::string > first_n_tokens( const std::string& fn, size_t n = 1 );
};

} // vidtk

#endif

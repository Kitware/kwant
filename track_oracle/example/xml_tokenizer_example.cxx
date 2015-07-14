/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <track_oracle/xml_tokenizer.h>

#include <logger/logger.h>


using std::istringstream;
using std::string;
using std::vector;


#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __vidtk_logger_auto_xml_tokenizer_example_cxx__
VIDTK_LOGGER("xml_tokenizer_example_cxx");


int main( int argc, char *argv[] )
{
  if (argc != 3)
  {
    LOG_INFO( "Usage: " << argv[0] << " xml-file  n-tokens");
    return EXIT_FAILURE;
  }

  istringstream iss( argv[2] );
  size_t n;
  if ( ! (iss >> n ))
  {
    LOG_ERROR( "Couldn't extract n-tokens from '" << argv[2] << "'");
    return EXIT_FAILURE;
  }

  vector< string > tokens = vidtk::xml_tokenizer::first_n_tokens( argv[1], n );
  LOG_INFO( "Got " << tokens.size() << " tokens:");
  for (size_t i=0; i<tokens.size(); ++i)
  {
    LOG_INFO( i << ":\t '" << tokens[i] << "'");
  }
}

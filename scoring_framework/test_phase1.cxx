/*ckwg +5
 * Copyright 2010 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <iostream>
#include <string>
#include <sstream>

#include <testlib/testlib_test.h>

#include <track_oracle/file_formats/track_kw18/track_kw18.h>
#include <track_oracle/file_formats/track_kw18/track_kw18_reader.h>

#include <scoring_framework/score_phase1.h>


using std::istringstream;
using std::string;

using namespace vidtk;

const string track_1 =
  "1 4 1  5 5  0 1  5 5  3 3 7 7   16 5 5 0  10\n";

const string track_2 =
  "1 4 2  5 6  0 1  5 6  3 4 7 8   16 5 6 0  11\n"
  "1 4 3  5 7  0 1  5 7  3 5 7 9   16 5 7 0  12\n"
  "1 4 4  5 8  0 0  5 8  3 6 7 10  16 5 8 0  13\n";


int
test_t1_t1()
{
  testlib_test_start( "testOneFrameToSelf" );
  istringstream iss( track_1 );
  track_list_type t;
  track_kw18_reader::read( iss, t );

  TEST( "track_1 has one track", (t.size() == 1) )
  track2track_phase1 p1;
  p1.compute( t, t );
  TEST( "t2t has one entry", (p1.t2t.size() == 1) );
  return testlib_test_summary();
}

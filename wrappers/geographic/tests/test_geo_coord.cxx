/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <testlib/testlib_test.h>
#include <geographic/geo_coords.h>

int main(int /*argc*/, char ** /*argv*/)
{
  testlib_test_start( "geo_coords" );
  vidtk::geographic::geo_coords c;

  TEST("Unset coords are invalid", c.is_valid(), false);

  c.reset(37.4, 28.9);
  TEST("37.4 28.9 is valid", c.is_valid(), true);

  c.reset(vidtk::geographic::INVALID_LAT_LON, vidtk::geographic::INVALID_LAT_LON);
  TEST("444 444 is invalid", c.is_valid(), false);

  return testlib_test_summary();
}

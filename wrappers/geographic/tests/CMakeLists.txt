add_executable( test_geo_coord
  test_geo_coord.cxx
)

target_link_libraries( test_geo_coord
  kwiver_geographic
  testlib
)

add_test( NAME geo_coord COMMAND $<TARGET_FILE:test_geo_coord> )

find_package(GeographicLib REQUIRED)
include_directories(SYSTEM ${GeographicLib_INCLUDE_DIR})

add_library( vidtk_geographic
  geo_coords.h  geo_coords.cxx
)
target_link_libraries(vidtk_geographic ${GeographicLib_LIBRARY})
install(TARGETS vidtk_geographic
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)

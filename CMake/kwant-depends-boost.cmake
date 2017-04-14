# Required Boost external dependency
find_package(Boost 1.55 REQUIRED
  COMPONENTS
    date_time
    system
    )

add_definitions(-DBOOST_ALL_NO_LIB)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
###
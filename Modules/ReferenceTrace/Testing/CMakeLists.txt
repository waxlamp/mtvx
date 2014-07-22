project(MTVX_REFTRACE_TESTING)
cmake_minimum_required(VERSION 2.8)

set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/testing")

# Timed reader test.
add_executable(timed-reader-test
  timed-reader-test.cpp
)

target_link_libraries(timed-reader-test
  mtvx-reftrace
  mtrtools
)

# Striped quad test.
add_executable(striped-quad-test
  striped-quad-test.cpp
)

target_link_libraries(striped-quad-test
  mtvx-reftrace
  mtrtools
)

# Region display test.
add_executable(region-display-test
  region-display-test.cpp
)

target_link_libraries(region-display-test
  mtvx-reftrace
)

# Cache display test.
add_executable(cacheset-display-test
  cacheset-display-test.cpp
)

target_link_libraries(cacheset-display-test
  daly
  mtvx-reftrace
)

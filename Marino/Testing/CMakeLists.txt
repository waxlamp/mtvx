project(MTVX_MARINO_TESTING)
cmake_minimum_required(VERSION 2.8)

set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/testing")

# Marino test.
add_executable(marino-test
  marino-test.cpp
)

target_link_libraries(marino-test
  mtvx-core
  mtvx-marino
  mtvx-reftrace
)

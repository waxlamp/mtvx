project(MTVX_APPLICATIONS_120)
cmake_minimum_required(VERSION 2.8)

set(_120_SOURCES
  120.cpp
)

add_executable(120
  ${_120_SOURCES}
)

target_link_libraries(120
  daly
  mtvx-core
)

project(MTVX_APPLICATIONS_WAXLAMP)
cmake_minimum_required(VERSION 2.8)

set(WAXLAMP_SOURCES
  waxlamp.cpp
)

add_executable(waxlamp
  ${WAXLAMP_SOURCES}
)

target_link_libraries(waxlamp
  mtvx-core
  mtvx-reftrace
)

project(MTVX_APPLICATIONS_ALTERNATIVENATION)
cmake_minimum_required(VERSION 2.8)

set(ALT_NATION_SOURCES
  altnat.cpp
)

add_executable(altnat
  ${ALT_NATION_SOURCES}
)

target_link_libraries(altnat
  mtvx-core
  mtvx-marino
  mtvx-reftrace
)

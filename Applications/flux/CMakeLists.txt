project(MTVX_APPLICATIONS_CREDIT)
cmake_minimum_required(VERSION 2.8)

add_executable(misstypes
  misstypes.cpp
)

target_link_libraries(misstypes
  mtvx-core
)

# add_script(scripts/miss-types.sh)
# add_script(scripts/gen-caches.py)

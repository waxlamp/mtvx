cmake_minimum_required(VERSION 2.8)
project(NEW_CACHE_SIMULATOR)

add_library(mtvx-new-cache
  CacheLevel.cpp
  CacheLevel.h
  NewCache.cpp
  NewCache.h
  NewCacheConstructor.cpp
  NewCacheConstructor.h
  NewCacheSet.cpp
  NewCacheSet.h
  NewCacheSetConstructor.cpp
  NewCacheSetConstructor.h
)

target_link_libraries(mtvx-new-cache
  ${Boost_LIBRARIES}
  mtvx-core
)

add_executable(test-new-cache
  test-new-cache.cpp
)

target_link_libraries(test-new-cache
  mtvx-new-cache
)

add_executable(compare-caches
  compare-caches.cpp
)

target_link_libraries(compare-caches
  ${Boost_LIBRARIES}
  daly
  mtvx-core
  mtvx-new-cache
)

add_executable(single-level
  single-level.cpp
)

target_link_libraries(single-level
  daly
  mtvx-core
  mtvx-new-cache
)

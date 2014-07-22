###############################################################
## CacheSim
##
## Utilities for performing cache simulation.
## Written by A.N.M. Imroz Choudhury, June 28, 2007
###############################################################
PROJECT(CACHESIM)

include(${CMAKE_SOURCE_DIR}/cmake/FindTinyxml.cmake)
include_directories(${TINYXML_INCLUDE_DIRS})

###############################################################
## Sources
###############################################################
SET(CACHESIM_SRC
  Cache.cpp
  Cache.h
  CacheSet.cpp
  CacheSet.h
  CacheConstructor.cpp
  CacheConstructor.h
  CacheSetConstructor.cpp
  CacheSetConstructor.h
)

###############################################################
## Create the cache simulation library
###############################################################
ADD_LIBRARY(daly ${CACHESIM_SRC})
target_link_libraries(daly
  ${Boost_LIBRARIES}
  ${TINYXML_LIBRARIES}
)

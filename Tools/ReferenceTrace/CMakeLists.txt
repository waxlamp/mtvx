project(MTRTOOLS)
cmake_minimum_required(VERSION 2.8)

include_directories(
  ${TINYXML_INCLUDE_DIRS}
)

# mtrtools library
add_library(mtrtools
  mtrtools.cpp
  mtrtools.h
)

target_link_libraries(mtrtools
  ${Boost_LIBRARIES}
  ${TINYXML_LIBRARIES}
)

# registrar library
add_library(mtrregistration
  registration.cpp
  registration.h
)

# build the human-readable trace printer program.
add_executable(mtrcat
  mtrcat.cpp
)

target_link_libraries(mtrcat
  mtrtools
  mtvx-reftrace
)

# build the trace filter program.
add_executable(mtrfilter
  mtrfilter.cpp
)

target_link_libraries(mtrfilter
  mtrtools
  mtvx-core
)

# build the specialized trace filter program.
add_executable(mtrstackfilter
  mtrstackfilter.cpp
)

target_link_libraries(mtrstackfilter
  mtrtools
  mtvx-core
)

add_executable(maketrace
  maketrace.cpp
)

target_link_libraries(maketrace
  mtrtools
  mtvx-core
)

# build the converter program
add_executable(mtrconvert
  mtrconvert.cpp
)

target_link_libraries(mtrconvert
  mtrtools
  mtvx-core
)

if(PIN_FOUND)
   # build the trace facility (with a custom makefile, since Pin has some
   # confusing method of compiling and linking
   add_custom_command(
     OUTPUT aeon.cpp Makefile.aeon
     COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/aeon.cpp aeon.cpp
     COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.aeon Makefile.aeon
     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/aeon.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.aeon
     )

  add_custom_target(
    aeon.so ALL make PIN_HOME=${PIN_HOME} AEON_INCLUDE_DIRS=${CMAKE_SOURCE_DIR} -f Makefile.aeon
    DEPENDS aeon.cpp Makefile.aeon
  )

  add_custom_command(
    TARGET aeon.so
    POST_BUILD
    COMMAND mv aeon.so ${CMAKE_BINARY_DIR}/bin
  )
endif(PIN_FOUND)

# Copy the linemap shell script to the output
# add_custom_command(
#   OUTPUT linemap.sh
#   COMMAND cp linemap.sh ${CMAKE_BINARY_DIR}/bin
#   DEPENDS linemap.sh
#   COMMENT hello
# )

add_custom_target(linemap.sh
  ALL
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/linemap.sh ${CMAKE_BINARY_DIR}/bin
  DEPENDS linemap.sh
)

add_custom_target(aeon
  ALL
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/aeon ${CMAKE_BINARY_DIR}/bin
  DEPENDS aeon
)

add_custom_target(mtrcrop
  ALL
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/mtrcrop ${CMAKE_BINARY_DIR}/bin
  DEPENDS mtrcrop
)

add_custom_target(mtrdata
  ALL
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/mtrdata ${CMAKE_BINARY_DIR}/bin
  DEPENDS mtrdata
)

add_custom_target(mtr2addr
  ALL
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/mtr2addr ${CMAKE_BINARY_DIR}/bin
  DEPENDS mtr2addr
)

# configure_file(linemap.sh ${CMAKE_BINARY_DIR}/bin COPYONLY)

# Build the testing executables.
subdirs(
  Testing
)

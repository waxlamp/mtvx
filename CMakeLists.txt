project(MTVX)
cmake_minimum_required(VERSION 2.8)

# Function for sending a script into the binary directory.
macro(add_script SCRIPT)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT}
    ${EXECUTABLE_OUTPUT_PATH}
    COPYONLY
  )
endmacro(add_script)

# Get Qt.
find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
include(${QT_USE_FILE})

# Get Boost.
# find_package(Boost 1.45 COMPONENTS filesystem iostreams system REQUIRED)
find_package(Boost 1.41 COMPONENTS filesystem iostreams system REQUIRED)

message(${Boost_LIBRARIES})

# Get OpenGL (this will include GLU).
find_package(OpenGL REQUIRED)

# Get Protobuf.
find_package(Protobuf REQUIRED)

# Get TinyXML.
include(${CMAKE_SOURCE_DIR}/cmake/FindTinyxml.cmake)

# Get the Pin install directory.
include(${CMAKE_SOURCE_DIR}/cmake/FindPin.cmake)

# Set output directories.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "Output directory for all executables.")
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "Output directory for all libraries.")

# Build shared libraries by default
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries.")

# Set the include directories for all builds.
include_directories(
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${QT_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${PROTOBUF_INCLUDE_DIR}
)

# Build the component libraries and the applications.
subdirs(
  Applications
  Core
  Marino
  # Modules/Lentil
  Modules/ReferenceTrace
  Tools/CacheSimulator
  Tools/NewCacheSimulator
  Tools/ReferenceTrace
)

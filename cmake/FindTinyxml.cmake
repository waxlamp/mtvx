# Find the TinyXml library.
#
# Defines:
# TINYXML_FOUND
# TINYXML_INCLUDE_DIRS
# TINYXML_LIBRARIES

include(FindPackageHandleStandardArgs)

set(TINYXML_PATH "/usr" CACHE PATH "install location for tinyxml")

find_path(TINYXML_INCLUDE_DIR
  NAMES tinyxml.h
  PATHS ${TINYXML_PATH}/include
)

find_library(TINYXML_LIBRARY
  NAMES tinyxml
  PATHS ${TINYXML_PATH}/lib
)

set(TINYXML_INCLUDE_DIRS
  ${TINYXML_INCLUDE_DIR}
)

set(TINYXML_LIBRARIES
  ${TINYXML_LIBRARY}
)

find_package_handle_standard_args(TINYXML DEFAULT_MSG TINYXML_LIBRARIES TINYXML_INCLUDE_DIRS)

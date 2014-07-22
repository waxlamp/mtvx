# Find the Pin components.
#
# Defines:
# PIN_FOUND
# PIN_HOME
# PIN_INCLUDE_DIRS

include(FindPackageHandleStandardArgs)

# find_path(PIN_INCLUDE_DIR
#   NAMES pin.H
#   PATHS /opt/pin/source/include
# )

find_path(PIN_HOME
  PATHS /opt/pin
)

set(PIN_INCLUDE_DIR ${PIN_HOME}/source/include)

find_package_handle_standard_args(PIN DEFAULT_MSG PIN_INCLUDE_DIRS)

if(PIN_HOME)
  set(PIN_FOUND true)
  set(PIN_INCLUDE_DIRS
    ${PIN_HOME}
  )
endif(PIN_HOME)

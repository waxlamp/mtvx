project(MTVX_TESTING)

set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/testing")

# Widget panel test.
add_executable(widget-panel-test
  widget-panel-test.cpp
)

target_link_libraries(widget-panel-test
  mtvx-core
)

# Dataflow test.
add_executable(dataflow-test
  dataflow-test.cpp
)

target_link_libraries(widget-panel-test
  mtvx-core
)

# Child widget test.
add_executable(child-widget-test
  child-widget-test.cpp
)

target_link_libraries(child-widget-test
  mtvx-core
)

# Timer test.
add_executable(timer-test
  timer-test.cpp
)

target_link_libraries(timer-test
  mtvx-core
)

# Widget animation test.
add_executable(widget-animation-test
  widget-animation-test.cpp
)

target_link_libraries(widget-animation-test
  mtvx-core
)

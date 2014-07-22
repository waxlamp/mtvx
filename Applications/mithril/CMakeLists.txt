project(MTVX_APPLICATIONS_MITHRIL)
cmake_minimum_required(VERSION 2.8)

set(MITHRIL_SOURCES
  mithril.cpp
)

# protobuf_generate_cpp(
#   MITHRIL_PROTO_SOURCES
#   MITHRIL_PROTO_HEADERS
#   Subprogram.proto
#   Type.proto
#   Variable.proto
# )

add_executable(mithril
  # ${MITHRIL_PROTO_HEADERS}
  # ${MITHRIL_PROTO_SOURCES}
  ${MITHRIL_SOURCES}
)

target_link_libraries(mithril
  mtvx-core
)

add_executable(addup
  addup.cpp
)

target_link_libraries(addup
  mtrregistration
)

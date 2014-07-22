project(MTVX_MARINO)
cmake_minimum_required(VERSION 2.8)

# protoc(Color.proto)
# protoc(Memento.proto)
# protoc(RegionRenderer.proto)

protobuf_generate_cpp(
  MARINO_PROTO_SOURCES
  MARINO_PROTO_HEADERS
  Color.proto
  Memento.proto
  RegionRenderer.proto
)

# set(marino_sources
#   Color.pb.cc
#   Color.pb.h
#   DeltaMementoRecorder.cpp
#   DeltaMementoRecorder.h
#   Memento.pb.cc
#   Memento.pb.h
#   RegionRenderer.pb.cc
#   RegionRenderer.pb.h
# )

set(marino_sources
  ${MARINO_PROTO_SOURCES}
  ${MARINO_PROTO_HEADERS}
  DeltaMementoRecorder.cpp
  DeltaMementoRecorder.h
)

add_library(mtvx-marino
  ${marino_sources}
)

target_link_libraries(mtvx-marino
  ${PROTOBUF_LIBRARY}
  pthread
)

# Build testing executables.
subdirs(
  Testing
)

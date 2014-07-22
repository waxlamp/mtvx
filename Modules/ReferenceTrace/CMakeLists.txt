project(MTVX_REFTRACE)
cmake_minimum_required(VERSION 2.8)

# protobuf_generate_cpp(
#   REFTRACE_PROTO_SOURCES
#   REFTRACE_PROTO_HEADERS
#   Animation/FrameDump.proto
# )

set(MTVX_REFTRACE_SOURCES
  # ${REFTRACE_PROTO_HEADERS}
  # ${REFTRACE_PROTO_SOURCES}
  Animation/AccessGrouper.h
  Animation/CacheGrouper.cpp
  Animation/CacheGrouper.h
  Dataflow/CacheEventRenderCommand.h
  Dataflow/CacheEventRenderDirector.cpp
  Dataflow/CacheEventRenderDirector.h
  Dataflow/CacheStatusRenderCommand.h
  Dataflow/CacheStatusRenderDirector.h
  Dataflow/RecordRenderCommand.h
  Dataflow/RecordRenderDirector.h
  Graphics/CacheDisplay.cpp
  Graphics/CacheDisplay.h
  Graphics/CacheLevelGlyph.cpp
  Graphics/CacheLevelGlyph.h
  Graphics/CacheSetDisplay.cpp
  Graphics/CacheSetDisplay.h
  Graphics/CacheTemperatureDisplay.cpp
  Graphics/CacheTemperatureDisplay.h
  Graphics/RegionDisplay.cpp
  Graphics/RegionDisplay.h
  Networks/MulticacheReferenceTraceNetwork.h
  Networks/WaxlampNetwork.h
  Networks/WaxlampCacheNetwork.cpp
  Networks/WaxlampCacheNetwork.h
  # Networks/WaxlampFifoNetwork.cpp
  # Networks/WaxlampFifoNetwork.h
  Networks/ReferenceTraceNetwork.h
  Renderers/CacheRenderer.cpp
  Renderers/CacheRenderer.h
  Renderers/CacheTemperatureRenderer.cpp
  Renderers/CacheTemperatureRenderer.h
  Renderers/RegionRenderer.h
  UI/MulticacheReferenceTracePanel.cpp
  UI/MulticacheReferenceTracePanel.h
  UI/ReferenceTracePanel.cpp
  UI/ReferenceTracePanel.h
  UI/WaxlampPanel.cpp
  UI/WaxlampPanel.h
)

# Create reftrace library.
add_library(mtvx-reftrace
  ${MTVX_REFTRACE_SOURCES}
)

target_link_libraries(mtvx-reftrace
  ${BOOST_LIBRARIES}
  mtrtools
  mtvx-core
  mtvx-marino
)

# Create fdcat executable.
add_executable(fdcat
  Animation/fdcat.cpp
)

target_link_libraries(fdcat
  mtvx-reftrace
)

# Build the testing executables.
subdirs(
  Testing
)

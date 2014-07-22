project(MTVX_LENTIL)
cmake_minimum_required(VERSION 2.8)

# Get Teem.
include(FindTeem.cmake)
include_directories(
  ${TEEM_INCLUDE_DIR}
)

set(MTVX_LENTIL_SOURCES
  Dataflow/DataRetrievalCommander.h
  Dataflow/ParticleSelector.h
  Dataflow/ParticleStatusRenderCommand.h
  Graphics/LentilDataDisplay.h
  Graphics/ParticleGlyph.cpp
  Graphics/ParticleGlyph.h
  Networks/LentilNetwork.cpp
  Networks/LentilNetwork.h
  Renderers/LentilDataRenderer.cpp
  Renderers/LentilDataRenderer.h
  UI/LentilPanel.cpp
  UI/LentilPanel.h
)

# Create lentil library.
add_library(mtvx-lentil
  ${MTVX_LENTIL_SOURCES}
)

target_link_libraries(mtvx-lentil
  mtrtools
  mtvx-core
  ${Boost_LIBRARIES}
  ${TEEM_LIBRARY}
)

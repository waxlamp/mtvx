project(MTVX_CORE)
cmake_minimum_required(VERSION 2.8)

# Wrap Qt classes.
qt4_wrap_cpp(QT_SOURCES
  Dataflow/DeltaMementoReader.h
  Dataflow/TraceReader.h
  Dataflow/UpdateNotifier.h
  UI/WidgetAnimationPanel.h
)

# Create protocol buffer source files.
protobuf_generate_cpp(
  CORE_PROTO_SOURCES
  CORE_PROTO_HEADERS
  BlockStreamHeader.proto
  FrameDump.proto
  LexicalBlock.proto
  Subprogram.proto
  Type.proto
  Variable.proto
)

# Set project sources.
set(MTVX_CORE_SOURCES
  ${CORE_PROTO_HEADERS}
  ${CORE_PROTO_SOURCES}
  Animation/Animator.h
  Animation/ColorAnimator.h
  Animation/GlowAnimator.h
  Animation/Grouper.h
  Animation/ShapeGrouper.cpp
  Animation/ShapeGrouper.h
  Animation/PointToPointAnimator.h
  Animation/PolarPointAnimator.h
  Animation/PulseAnimator.h
  Dataflow/ArrayIndexer.h
  Dataflow/CacheAccessRecord.h
  Dataflow/CachePerformanceCounter.cpp
  Dataflow/CachePerformanceCounter.h
  # Dataflow/CacheSimulator.cpp
  Dataflow/CacheSimulator.h
  Dataflow/CacheStatusReport.h
  Dataflow/Consumer.h
  Dataflow/DeltaMementoReader.cpp
  Dataflow/DeltaMementoReader.h
  Dataflow/Filter.h
  Dataflow/Ground.h
  Dataflow/HitLevelCounter.cpp
  Dataflow/HitLevelCounter.h
  Dataflow/ItemSelector.h
  Dataflow/LineCacheMissCounter.cpp
  Dataflow/LineCacheMissCounter.h
  Dataflow/LineRecordFilter.h
  Dataflow/LineVisitCounter.cpp
  Dataflow/LineVisitCounter.h
  Dataflow/MemoryRecordFilter.h
  Dataflow/Producer.h
  Dataflow/Printer.h
  Dataflow/Repeater.h
  Dataflow/SetUtilizationCounter.cpp
  Dataflow/SetUtilizationCounter.h
  Dataflow/SignalRecordFilter.h
  Dataflow/TraceReader.cpp
  Dataflow/TraceSignal.h
  Dataflow/TraceWriter.cpp
  Dataflow/TraceWriter.h
  Dataflow/BlockStream/BlockStreamReader.cpp
  Dataflow/BlockStream/BlockStreamReader.h
  Dataflow/BlockStream/LRUStreamPool.h
  Color/Color.cpp
  Color/Color.h
  Color/ColorGenerator.cpp
  Color/ColorGenerator.h
  Color/ColorProfile.h
  Geometry/ArchimedeanSpiral.h
  Geometry/ParameterAffineRemapper.h
  Geometry/ParameterFunctionRemapper.h
  Geometry/Parametrized.h
  Geometry/ParametrizedCircle.h
  Geometry/Point.cpp
  Geometry/Point.h
  Geometry/RandomDisc.h
  Geometry/Vector.cpp
  Geometry/Vector.h
  Graphics/AlphaGlyph.cpp
  Graphics/AlphaGlyph.h
  Graphics/GaussianAlphaGlyph.cpp
  Graphics/GaussianAlphaGlyph.h
  Graphics/FadingPoint.cpp
  Graphics/FadingPoint.h
  Graphics/FadingRing.cpp
  Graphics/FadingRing.h
  Graphics/GLPoint.h
  Graphics/MotionBlur.cpp
  Graphics/MotionBlur.h
  Graphics/RowContainer.cpp
  Graphics/RowContainer.h
  Graphics/SineAlphaGlyph.cpp
  Graphics/SineAlphaGlyph.h
  Graphics/SolidRectangle.h
  Graphics/StripedQuad.cpp
  Graphics/StripedQuad.h
  Graphics/TextWidget.cpp
  Graphics/TextWidget.h
  Graphics/Tick.cpp
  Graphics/Tick.h
  Graphics/Widget.cpp
  Graphics/Widget.h
  Math/Interpolation.cpp
  Math/Interpolation.h
  UI/WidgetAnimationPanel.cpp
  UI/WidgetAnimationPanel.h
  UI/WidgetManipulationPanel.cpp
  UI/WidgetManipulationPanel.h
  UI/WidgetPanel.cpp
  UI/WidgetPanel.h
  Util/Boost.h
  Util/BoostForeach.h
  Util/BoostPointers.h
  Util/LevelComparator.h
  Util/Util.cpp
  Util/Util.h
  Util/Timing.h
  ${QT_SOURCES}
)

add_library(mtvx-core
  ${MTVX_CORE_SOURCES}
)

target_link_libraries(mtvx-core
  ${QT_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OPENGL_LIBRARIES}
  daly
  mtvx-marino # TODO(choudhury): can this dependency be somehow
              # avoided?
  mtrtools
)

add_executable(mtr2blockstream
  Dataflow/BlockStream/mtr2blockstream.cpp
)

target_link_libraries(mtr2blockstream
  mtvx-core
)

add_executable(blockstreamdump
  Dataflow/BlockStream/blockstreamdump.cpp
)

target_link_libraries(blockstreamdump
  mtvx-core
)

add_executable(blockstreamrecon
  Dataflow/BlockStream/blockstreamrecon.cpp
)

target_link_libraries(blockstreamrecon
  mtvx-core
)

# Build the testing executables.
subdirs(
  Testing
)

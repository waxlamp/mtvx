project(MTVX_APPLICATIONS)
cmake_minimum_required(VERSION 2.8)

qt4_wrap_cpp(QT_SOURCES
  MTVMainWindow.h
)

qt4_add_resources(QT_RESOURCES
  icons.qrc
)

add_executable(mtvx
  mtvx.cpp
  MTVMainWindow.h
  MTVMainWindow.cpp
  ${QT_SOURCES}
  ${QT_RESOURCES}
)

target_link_libraries(mtvx
  ${QT_LIBRARIES}
  mtvx-core
  # mtvx-lentil
  mtvx-reftrace
)

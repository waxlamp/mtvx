project(MTRTOOLS_TESTING)
cmake_minimum_required(VERSION 2.8)

set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/testing")

# build the example trace writer.
add_executable(make-trace
  make-trace.cpp
)

target_link_libraries(make-trace
  mtrtools
)

# build the example "real world" program for reference trace testing.
add_executable(array-walk
  array-walk.c
)

add_executable(registrar-example
  registrar-example.cpp
)

target_link_libraries(registrar-example
  mtrregistration
)

# build the sorting test program.
add_executable(sort
  sort.cpp
)

target_link_libraries(sort
  mtrregistration
)

add_executable(sort-array
  sort-array.cpp
)

target_link_libraries(sort-array
  mtrregistration
)

add_executable(matmult
  matmult/matmult.cpp
  matmult/matmult.h
  matmult/runmatmult.cpp
)

target_link_libraries(matmult
  mtrregistration
)

add_executable(matmult-transpose
  matmult/matmult-transpose.cpp
  matmult/matmult.h
  matmult/runmatmult.cpp
)

target_link_libraries(matmult-transpose
  mtrregistration
)

add_executable(mergesort
  mergesort.cpp
)

target_link_libraries(mergesort
  mtrregistration
)

add_executable(insertionsort
  insertionsort.cpp
)

target_link_libraries(insertionsort
  mtrregistration
)

add_executable(triangle-render
  triangle-render/triangle-render.cpp
)

target_link_libraries(triangle-render
  mtrregistration
)

add_executable(diffusion
  diffusion/diffusion.cpp
)

target_link_libraries(diffusion
  mtrregistration
)

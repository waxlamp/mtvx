project(MTVX_APPLICATIONS_CREDIT)
cmake_minimum_required(VERSION 2.8)

add_executable(credit
  credit.cpp
)

target_link_libraries(credit
  daly
  mtvx-core
)

add_executable(debit
  debit.cpp
)

target_link_libraries(debit
  mtvx-core
  mtvx-new-cache
)

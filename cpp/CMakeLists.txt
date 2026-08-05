cmake_minimum_required(VERSION 3.16)
project(ytta_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(ytta_m0_smoke src/smoke/smoke.cpp)

add_executable(ytta_m1_tests tests/m1_blocks_test.cpp)
target_include_directories(ytta_m1_tests PRIVATE src/m1)
target_link_libraries(ytta_m1_tests PRIVATE Threads::Threads)

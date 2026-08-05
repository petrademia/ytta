cmake_minimum_required(VERSION 3.16)
project(ytta_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(ytta_m0_smoke src/smoke/smoke.cpp)

add_executable(ytta_m1_tests tests/m1_blocks_test.cpp)
target_include_directories(ytta_m1_tests PRIVATE src/m1)
target_link_libraries(ytta_m1_tests PRIVATE Threads::Threads)

add_library(ytta_m2_lib STATIC src/m2/matching_engine.cpp)
target_include_directories(ytta_m2_lib PUBLIC src/m2)

add_executable(ytta_m2_tests tests/m2_engine_test.cpp)
target_link_libraries(ytta_m2_tests PRIVATE ytta_m2_lib)

add_library(ytta_m3_lib STATIC src/m3/net/framing.cpp src/m3/net/tcp.cpp)
target_include_directories(ytta_m3_lib PUBLIC src/m3)

add_executable(ytta_m3_tests tests/m3_net_test.cpp)
target_link_libraries(ytta_m3_tests PRIVATE ytta_m3_lib Threads::Threads)

add_library(ytta_m4_lib STATIC src/m4/pipeline.cpp)
target_include_directories(ytta_m4_lib PUBLIC src/m4 src/m2)
target_link_libraries(ytta_m4_lib PUBLIC ytta_m2_lib)

add_executable(ytta_m4 src/m4_main.cpp)
target_link_libraries(ytta_m4 PRIVATE ytta_m4_lib)

add_executable(ytta_m4_tests tests/m4_pipeline_test.cpp)
target_link_libraries(ytta_m4_tests PRIVATE ytta_m4_lib)

add_library(ytta_m5_lib STATIC src/m5/strategy.cpp)
target_include_directories(ytta_m5_lib PUBLIC src/m5 src)

add_executable(ytta_m5_tests tests/m5_strategy_test.cpp)
target_link_libraries(ytta_m5_tests PRIVATE ytta_m5_lib)

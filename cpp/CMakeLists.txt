cmake_minimum_required(VERSION 3.16)
project(ytta_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(ytta_thread_demo
    src/thread_main.cpp
    src/thread.cpp
)
target_include_directories(ytta_thread_demo PRIVATE include)
target_link_libraries(ytta_thread_demo PRIVATE Threads::Threads)

add_executable(ytta_mem_pool_demo
    src/mem_pool.cpp
)
target_include_directories(ytta_mem_pool_demo PRIVATE include)

set(YTTA_V0_SOURCES
    src/v0/book.cpp
    src/v0/matching_engine.cpp
    src/v0/tick_source.cpp
    src/v0/strategy.cpp
    src/v0/pipeline.cpp
    src/v0/net/framing.cpp
    src/v0/net/tcp.cpp
    src/v0/engine_server.cpp
    src/v0/tcp_order_gateway.cpp
)

add_library(ytta_v0_lib STATIC ${YTTA_V0_SOURCES})
target_include_directories(ytta_v0_lib PUBLIC src/v0)

add_executable(ytta_v0_tests
    tests/v0_book_test.cpp
)
target_link_libraries(ytta_v0_tests PRIVATE ytta_v0_lib)

add_executable(ytta_v0_spsc_tests
    tests/v0_spsc_test.cpp
)
target_link_libraries(ytta_v0_spsc_tests PRIVATE ytta_v0_lib)

add_executable(ytta_v0_net_tests
    tests/v0_net_test.cpp
)
target_link_libraries(ytta_v0_net_tests PRIVATE ytta_v0_lib)

add_executable(ytta_v0
    src/v0_main.cpp
)
target_link_libraries(ytta_v0 PRIVATE ytta_v0_lib)

add_executable(ytta_engine
    src/v0_engine_main.cpp
)
target_link_libraries(ytta_engine PRIVATE ytta_v0_lib)

add_executable(ytta_client
    src/v0_client_main.cpp
)
target_link_libraries(ytta_client PRIVATE ytta_v0_lib)

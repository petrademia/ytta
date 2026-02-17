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

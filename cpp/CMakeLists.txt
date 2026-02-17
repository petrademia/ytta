cmake_minimum_required(VERSION 3.16)
project(ytta_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(ytta_cpp
    src/main.cpp
    src/thread.cpp
)
target_include_directories(ytta_cpp PRIVATE include)
target_link_libraries(ytta_cpp PRIVATE Threads::Threads)

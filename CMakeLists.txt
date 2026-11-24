cmake_minimum_required(VERSION 3.16)
project(RookEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib REQUIRED)

add_executable(engine_demo
    src/main.cpp
)

target_include_directories(engine_demo PRIVATE src)
target_link_libraries(engine_demo PRIVATE raylib m)

cmake_minimum_required(VERSION 3.10)
project(components)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(base)
add_subdirectory(pan)
add_subdirectory(msva)

cmake_minimum_required(VERSION 3.10)
project(libpan C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(libPan STATIC)
target_sources(libPan PRIVATE src/libPan.c)
target_include_directories(libPan PUBLIC inc/)

add_executable(pan)
target_sources(pan PRIVATE src/standalone.c)
target_link_libraries(pan PRIVATE libPan)

cmake_minimum_required(VERSION 3.13)
project(iprof)

option(BUILD_TEST "Build the test program" OFF)

add_subdirectory(src/)

target_include_directories(iprof PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)

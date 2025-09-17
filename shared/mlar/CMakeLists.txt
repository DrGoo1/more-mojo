cmake_minimum_required(VERSION 3.15)
project(MLAR VERSION 3.1.1)
add_library(mlar INTERFACE)
target_include_directories(mlar INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

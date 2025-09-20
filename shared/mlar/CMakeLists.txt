cmake_minimum_required(VERSION 3.15)
project(MLAR VERSION 3.1.1)

# Define MLAR library target
add_library(mlar STATIC
    MLAR.cpp
    MLAR.h
)

# Set include directories
target_include_directories(mlar
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

message(STATUS "MLAR v3.1.1 module configured")

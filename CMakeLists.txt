cmake_minimum_required(VERSION 2.8)
project(parasol)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(include)

set(LIBRARY_SOURCE_FILES
    )

add_library(parasol ${LIBRARY_SOURCE_FILES})
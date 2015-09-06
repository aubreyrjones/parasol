cmake_minimum_required(VERSION 2.8)
project(parasol)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(include)


add_executable(lemon lemon/lemon.c)

#set(LIBRARY_SOURCE_FILES)

#add_library(parasol ${LIBRARY_SOURCE_FILES})

set(COMPILER_SOURCE_FILES
    prslc/ParasolPT.h
    prslc/ParasolPT.cpp)

add_executable(prslc ${COMPILER_SOURCE_FILES})

cmake_minimum_required(VERSION 2.8)
project(parasol)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(include)


add_executable(lemon lemon/lemon.c)

#set(LIBRARY_SOURCE_FILES)

#add_library(parasol ${LIBRARY_SOURCE_FILES})

set(LEMON_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/lemon")

add_custom_command(OUTPUT "${CMAKE_SOURCE_DIR}/prslc/prsl_grammar.c"
        COMMAND "${CMAKE_BINARY_DIR}/lemon" "-T${LEMON_DIR}/lempar.c" "${CMAKE_SOURCE_DIR}/prslc/prsl_grammar.y"
        DEPENDS lemon
        VERBATIM
    )

set(COMPILER_SOURCE_FILES
    prslc/ParasolPT.h
    prslc/ParasolPT.cpp
    prslc/main.cpp
    prslc/prsl_grammar.c)

add_executable(prslc ${COMPILER_SOURCE_FILES})

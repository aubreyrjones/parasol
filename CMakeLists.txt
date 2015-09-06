cmake_minimum_required(VERSION 2.8)
project(parasol)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(include)


add_executable(lemon lemon/lemon.c)

#set(LIBRARY_SOURCE_FILES)

#add_library(parasol ${LIBRARY_SOURCE_FILES})

set(LEMON_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/lemon")

set(PRSLC_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/prslc")

add_custom_command(OUTPUT "${PRSLC_DIR}/prsl_grammar.c" "${PRSLC_DIR}/prsl_grammar.h" "${PRSLC_DIR}/prsl_grammar.out"
        COMMAND "${CMAKE_BINARY_DIR}/lemon" "-T${LEMON_DIR}/lempar.c" "${PRSLC_DIR}/prsl_grammar.y"
        DEPENDS lemon prslc/prsl_grammar.y
        VERBATIM
    )

set(COMPILER_SOURCE_FILES
    prslc/ParasolPT.h
    prslc/ParasolPT.cpp
    prslc/main.cpp)

SET_SOURCE_FILES_PROPERTIES(prslc/prsl_grammar.c PROPERTIES LANGUAGE CXX )

add_executable(prslc ${COMPILER_SOURCE_FILES} "${PRSLC_DIR}/prsl_grammar.c")

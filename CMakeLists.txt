cmake_minimum_required(VERSION 2.8)
project(parasol)

# sorry folks, it's gotta be c++11. If you aren't paying me, I'm not going back!
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# this brings in our partial, headers-only subset of boost
include_directories(include)


# first we build the lemon parser generator (this can be done offline or out of build)
add_executable(lemon lemon/lemon.c)
set(LEMON_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/lemon")
set(PRSLC_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/prslc")

# now we use lemon to build the parser from the grammar file
add_custom_command(OUTPUT "${PRSLC_DIR}/prsl_grammar.c" "${PRSLC_DIR}/prsl_grammar.h" "${PRSLC_DIR}/prsl_grammar.out"
        COMMAND "${CMAKE_CURRENT_BINARY_DIR}/lemon" "-T${LEMON_DIR}/lempar.c" "${PRSLC_DIR}/prsl_grammar.y"
        DEPENDS lemon prslc/prsl_grammar.y
        VERBATIM)
# it's a .c file, 'cause that's what lemon outputs. Compile it as c++ anyway.
SET_SOURCE_FILES_PROPERTIES(prslc/prsl_grammar.c PROPERTIES LANGUAGE CXX)


# build the compiler
set(COMPILER_SOURCE_FILES
    prslc/ParasolPT.h
    prslc/ParasolPT.cpp
    prslc/main.cpp)


add_executable(prslc ${COMPILER_SOURCE_FILES} "${PRSLC_DIR}/prsl_grammar.c")

cmake_minimum_required(VERSION 2.8)
project(parasol)

# sorry folks, it's gotta be c++11. If you aren't paying me, I'm not going back!
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wno-multichar -std=c++11")

# this brings in our partial, headers-only subset of boost
include_directories(include)

# ==========================
# build the runtime library.
# ==========================

# If you are using a precompiled compiler and DO NOT DYNAMICALLY GENERATE SHADERS FROM SOURCE,
# then just dump the library source files in your own build.
# If you expect to submit source code strings to create shaders at runtime (like in an editor, or a dynamic
# shader system), then you must also include the `prsl` target below.

# set(LIBRARY_SOURCE_FILES
#    )
# add_library(parasol STATIC ${LIBRARY_SOURCE_FILES})



# ===========================
# build the parasol compiler.
# ===========================

# first we build the lemon parser generator (this can be done offline or out of build)
add_executable(lemon lemon/lemon.c)
set(LEMON_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/lemon")
set(PRSLC_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/prslc")

# now we use lemon to build the parser from the grammar file

# I add these files to the source tree so that my IDE doesn't complain about missing symbols, and to make
# the build easier to describe. This can also be done offline or out of build.
add_custom_command(OUTPUT "${PRSLC_DIR}/prsl_grammar.c" "${PRSLC_DIR}/prsl_grammar.h" "${PRSLC_DIR}/prsl_grammar.out"
        COMMAND "${CMAKE_CURRENT_BINARY_DIR}/lemon" "-T${LEMON_DIR}/lempar.c" "${PRSLC_DIR}/prsl_grammar.y"
        DEPENDS lemon prslc/prsl_grammar.y prslc/Token.h
        VERBATIM)
# it's a .c file, 'cause that's what lemon outputs. Compile it as c++ anyway, because the parser actions are
# c++ code.
SET_SOURCE_FILES_PROPERTIES(prslc/prsl_grammar.c PROPERTIES LANGUAGE CXX)


# build the compiler
set(COMPILER_SOURCE_FILES
        "${PRSLC_DIR}/prsl_grammar.c"
        prslc/ParasolAST.h
        prslc/ParasolAST.cpp
        prslc/Lexer.cpp
        prslc/Lexer.h
        prslc/Token.cpp
        prslc/Token.h prslc/Parser.cpp
        prslc/Parser.h
        prslc/DOTVisitor.cpp
        prslc/DOTVisitor.h
        prslc/IDVisitor.cpp
        prslc/IDVisitor.h
        prslc/Visitor.cpp
        prslc/Visitor.h
        prslc/Compiler.cpp
        prslc/Compiler.h
        prslc/Types.cpp
        prslc/Types.h
        prslc/Builtins.cpp
        prslc/Builtins.h)

add_library(prsl STATIC ${COMPILER_SOURCE_FILES})

add_executable(prslc prslc/main.cpp)
target_link_libraries(prslc prsl)


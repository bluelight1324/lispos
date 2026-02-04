# ==============================================================================
# CMakeLists.txt - Lisp Compiler/Interpreter Build Configuration
# ==============================================================================

cmake_minimum_required(VERSION 3.16)
project(LispCompiler C)

# C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
if(MSVC)
    # MSVC flags
    add_compile_options(/W4 /WX-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    # GCC/Clang flags
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Source files
set(LISP_SOURCES
    src/main.c
    src/lisp.c
    src/lexer.c
    src/parser.c
    src/env.c
    src/eval.c
    src/primitives.c
    src/codegen.c
    src/debug.c
)

# Main executable
add_executable(lisp ${LISP_SOURCES})

# Include directory
target_include_directories(lisp PRIVATE src)

# Link math library on Unix
if(UNIX)
    target_link_libraries(lisp m)
endif()

# Install target
install(TARGETS lisp DESTINATION bin)

# ==============================================================================
# Optional: Generate parser with LALRGen (if available)
# ==============================================================================

find_program(LALRGEN_EXE LALRGen PATHS "${CMAKE_SOURCE_DIR}/../LALRGen/build" "${CMAKE_SOURCE_DIR}/../LALRGen")

if(LALRGEN_EXE)
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/src/synout0.cpp
        COMMAND ${LALRGEN_EXE} ${CMAKE_SOURCE_DIR}/src/lisp_grammar.y
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
        DEPENDS ${CMAKE_SOURCE_DIR}/src/lisp_grammar.y
        COMMENT "Generating parser tables with LALRGen"
    )
    add_custom_target(generate_parser DEPENDS ${CMAKE_SOURCE_DIR}/src/synout0.cpp)
endif()

# ==============================================================================
# Testing
# ==============================================================================

enable_testing()

# Simple test: run the REPL with a test expression
add_test(
    NAME test_basic
    COMMAND lisp -c "${CMAKE_SOURCE_DIR}/test/factorial.scm" -o "${CMAKE_BINARY_DIR}/factorial.asm"
)

# ==============================================================================
# Print configuration summary
# ==============================================================================

message(STATUS "")
message(STATUS "=== Lisp Compiler Configuration ===")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
if(LALRGEN_EXE)
    message(STATUS "LALRGen found: ${LALRGEN_EXE}")
else()
    message(STATUS "LALRGen not found (using hand-written parser)")
endif()
message(STATUS "")

cmake_minimum_required(VERSION 3.16)
project(LALRGen VERSION 2.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Compiler flags for MSVC
if(MSVC)
    add_compile_options(
        /W3
        /wd4018  # signed/unsigned mismatch
        /wd4101  # unreferenced local variable
        /wd4706  # assignment within conditional
        /wd4715  # not all paths return
        /wd4996  # deprecated functions
    )
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_WARNINGS
        LALRGEN
        WIN32
        _CONSOLE
        NO_MFC
    )
endif()

# Include directories
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Include")

# Source files
set(LALRGEN_SOURCES
    MAIN.c
    ACTS.c
    ASSORT.c
    FERR.c
    FIRST.c
    HASH.c
    HASHADD.c
    INPUT.c
    LEXYY.c
    ONFERR.c
    PRINTV.c
    PRNT.c
    SIGNON.c
    STOK.c
    YYCODE.c
    YYDOLLAR.c
    YYDRIVER.c
    YYOUT.c
    YYPATCH.c
    YYSTATE_clean.c
    ../Sets/SetLegacy.cpp
    ../Sets/virtualmem.cpp
)

# Create executable
add_executable(LALRGen ${LALRGEN_SOURCES})

target_include_directories(LALRGen PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Sets
    ${INCLUDE_DIR}
)

# Link libraries
if(WIN32)
    target_link_libraries(LALRGen PRIVATE
        odbc32
        odbccp32
    )
endif()

# Installation
install(TARGETS LALRGen
    RUNTIME DESTINATION bin
)

cmake_minimum_required(VERSION 3.0)

project(mgbase)

add_library(mgbase-header INTERFACE)
target_include_directories(mgbase-header
    INTERFACE ./include
    INTERFACE ./external
)
target_compile_definitions(mgbase-header
    INTERFACE "$<$<CONFIG:Debug>:MGBASE_DEBUG>"
)

add_library(mgbase-options INTERFACE)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Fujitsu")
    target_compile_options(mgbase-options INTERFACE
        -Xg
        -D GTEST_HAS_TR1_TUPLE=0
    )
else()
    target_compile_options(mgbase-options INTERFACE
        "-Wall"
        "-std=c++03"
        "-pedantic"
        "-Wextra"
        "-Wcast-qual"
        "-Wctor-dtor-privacy"
        "-Wdisabled-optimization"
        "-Wformat=2"
        "-Winit-self"
        "-Wmissing-declarations"
        "-Wmissing-include-dirs"
        "-Woverloaded-virtual"
        "-Wredundant-decls"
        "-Wshadow"
        "-Wsign-promo"
        "-Wstrict-overflow=5"
        "-Wswitch-default"
        "-Wno-variadic-macros"
        "$<$<CXX_COMPILER_ID:Clang>:-Wno-c++11-long-long>" # for mpi.h
        "$<$<CXX_COMPILER_ID:GNU>:-Wno-long-long>" # for mpi.h
    )
endif()


add_library(mgbase
    ./external/cppformat/format.cc
)
target_link_libraries(mgbase mgbase-header)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Fujitsu")
    target_compile_options(mgbase
        PRIVATE
        -Xg
        -include ${CMAKE_CURRENT_SOURCE_DIR}/include/mgbase/external/cppformat.hpp
    )
endif()

add_library(googletest
    ./external/googletest/googletest/src/gtest-all.cc
)
target_include_directories(googletest
    PUBLIC ./external/googletest/googletest
    PUBLIC ./external/googletest/googletest/include
)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Fujitsu")
    target_sources(googletest
        PRIVATE ./src/workaround/wcscasecmp.cpp
    )
    target_compile_options(googletest
        PRIVATE
        -Xg
        -include ${CMAKE_CURRENT_SOURCE_DIR}/src/workaround/wcscasecmp.h
        -D GTEST_HAS_TR1_TUPLE=0
    )
endif()

add_executable(mgbase-unittest
    unittest/unittest.cpp
    unittest/deferred.cpp
)

target_link_libraries(mgbase-unittest mgbase mgbase-options googletest)


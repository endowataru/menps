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

add_library(mgbase
    ./external/cppformat/format.cc
)
target_link_libraries(mgbase mgbase-header)

add_library(googletest
    ./external/googletest/googletest/src/gtest-all.cc
)
target_include_directories(googletest
    PUBLIC ./external/googletest/googletest
    PUBLIC ./external/googletest/googletest/include
)

add_executable(mgbase-unittest
    unittest/unittest.cpp
    unittest/deferred.cpp
)

target_link_libraries(mgbase-unittest mgbase googletest)


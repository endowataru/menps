cmake_minimum_required(VERSION 3.0)

project(mgbase)

add_library(mgbase-header INTERFACE)
target_include_directories(mgbase-header
    INTERFACE ./include
    INTERFACE ./external
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


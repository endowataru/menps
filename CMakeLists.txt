cmake_minimum_required(VERSION 3.0)

project(mgctx CXX)

add_library(mgctx INTERFACE)
target_include_directories(mgctx
    INTERFACE ./include
)
target_link_libraries(mgctx
    INTERFACE mgbase
    mgbase-options
)

add_executable(mgctx-unittest
    unittest/unittest.cpp
    unittest/context.cpp
)
target_link_libraries(mgctx-unittest
    mgctx
    googletest
)


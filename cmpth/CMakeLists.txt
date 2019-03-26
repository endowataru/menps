cmake_minimum_required(VERSION 3.0)

project(cmpth CXX)

add_library(cmpth INTERFACE)

target_include_directories(cmpth
    INTERFACE ./include
)

add_library(cmpth-options INTERFACE)

target_compile_options(cmpth-options INTERFACE
    -Wall -Wextra -g -O3 -std=c++11 -ftls-model=initial-exec
)


add_executable(cmpth-test
    ./example/test.cpp
)
target_link_libraries(cmpth-test
    cmpth
    cmpth-options
)


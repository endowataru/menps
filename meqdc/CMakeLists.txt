cmake_minimum_required(VERSION 3.0)

project(meqdc)

add_library(meqdc
    src/compile.cpp
)

target_include_directories(meqdc
    PUBLIC ./include
)
target_link_libraries(meqdc
    PUBLIC medev2
    PRIVATE mefdn-options
)


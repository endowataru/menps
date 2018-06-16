cmake_minimum_required(VERSION 3.0)

project(meuct)

add_library(meuct
    src/meuct.cpp
)

target_include_directories(meuct
    PUBLIC ./include
)
target_link_libraries(meuct
    PUBLIC medev2
    PRIVATE mefdn-options
)


cmake_minimum_required(VERSION 3.0)

project(medev2)

add_library(medev2 INTERFACE)

target_include_directories(medev2
    INTERFACE ./include
)
target_link_libraries(medev2
    INTERFACE mefdn
)


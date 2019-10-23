cmake_minimum_required(VERSION 3.1)

project(medsm3)

add_library(medsm3
    ./src/md3.cpp
)
target_include_directories(medsm3
    PUBLIC ./include
)
target_link_libraries(medsm3
    PUBLIC mecom2
    PUBLIC medsm2
    PRIVATE mefdn-options
    PUBLIC rt # TODO: Is this necessary?
)

add_executable(medsm3-ex
    ./example/ex.cpp
)
target_link_libraries(medsm3-ex
    PUBLIC medsm3
    PRIVATE mefdn-options
    PRIVATE medsm2-exec-options
)


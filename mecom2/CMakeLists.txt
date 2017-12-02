cmake_minimum_required(VERSION 3.0)

project(mecom2)

add_executable(mecom2-test
    ./unittest/unittest.cpp
    ./unittest/rpc.cpp
    ./unittest/rma.cpp
    #./src/compile.cpp
)
target_include_directories(mecom2-test
    PUBLIC ./include
)
target_link_libraries(mecom2-test
    PUBLIC medev2
    PRIVATE mefdn-options
    PRIVATE googletest
)


cmake_minimum_required(VERSION 3.0)

project(mecom2)

add_library(mecom2 INTERFACE)

target_include_directories(mecom2
    INTERFACE ./include
)
target_link_libraries(mecom2
    INTERFACE medev2
    INTERFACE $<$<BOOL:${MECOM2_USE_MEUCT}>:meuct>
)


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
    PUBLIC mecom2
    PRIVATE mefdn-options
    PRIVATE googletest
)


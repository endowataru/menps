cmake_minimum_required(VERSION 3.0)

project(mecom2)

option(MECOM2_RMA_ENABLE_LOOPBACK "Enable loopback RMA read/write" FALSE)

option(MECOM2_USE_MEUCT "Use meuct for mecom2" FALSE)

option(MECOM2_USE_WORKER_LOCAL_ALLOCATOR "Use worker-local RMA allocator" TRUE)

option(MECOM2_AVOID_MPI_WAIT "Avoid using MPI_Wait()" FALSE)

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/menps/mecom2/config.h)


add_library(mecom2 INTERFACE)

target_include_directories(mecom2
    INTERFACE ./include
    INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(mecom2
    INTERFACE meqdc
    INTERFACE $<$<BOOL:${MECOM2_USE_MEUCT}>:meuct>
)


add_executable(mecom2-test
    ./unittest/unittest.cpp
    #./unittest/rpc.cpp
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


cmake_minimum_required(VERSION 3.0)

project(mecom2)

option(MECOM2_RMA_ENABLE_LOOPBACK "Enable loopback RMA read/write" FALSE)

option(MECOM2_UCT_RMA_ENABLE_WORKER_LOCK
    "Use locks for UCT workers (necessary for direct UCT interface)" OFF)
option(MECOM2_UCT_RMA_ENABLE_EXPLICIT_PROGRESS
    "Enable explicit progress for UCT workers" OFF)
option(MECOM2_UCT_RMA_ENABLE_CONCURRENT_LIMIT
    "Enable concurrent limit for UCT workers" OFF)
option(MECOM2_UCT_ENABLE_PARALLEL_INIT_WORKER_SET
    "Enable parallel initialization of UCT workers (may cause bugs)" OFF)

option(MECOM2_USE_WORKER_LOCAL_ALLOCATOR "Use worker-local RMA allocator" TRUE)

option(MECOM2_AVOID_MPI_WAIT "Avoid using MPI_Wait()" FALSE)

set(MECOM2_RMA_UCT_GET_ZCOPY_SIZE "64" CACHE STRING "Threshold size for uct_ep_get_zcopy()")

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
)
install(DIRECTORY include/ DESTINATION include)


add_executable(mecom2-test
    ./tests/test.cpp
)
target_link_libraries(mecom2-test
    PUBLIC mecom2
    PRIVATE mefdn-options
    PUBLIC doctest
)

enable_testing()

add_test(
    NAME mecom2-test
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_sweep.bash $<TARGET_FILE:mecom2-test>
)



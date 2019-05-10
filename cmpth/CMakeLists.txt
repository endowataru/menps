cmake_minimum_required(VERSION 3.1)

project(cmpth CXX)

option(CMPTH_ENABLE_MTH "Build MassiveThreads" ON)

add_library(cmpth INTERFACE)

target_include_directories(cmpth
    INTERFACE ./include
)


set(CMPTH_GLOBAL_CFLAGS -ftls-model=initial-exec)
# CMPTH_GLOBAL_CFLAGS will be used in massivethread.cmkae

add_library(cmpth-options INTERFACE)

target_compile_options(cmpth-options INTERFACE
    -Wall -Wextra ${CMPTH_GLOBAL_CFLAGS}
)


add_executable(cmpth-test
    ./example/test.cpp
)
target_link_libraries(cmpth-test
    cmpth
    cmpth-options
)

if (${CMPTH_ENABLE_MTH})
    include(./cmake/massivethreads.cmake)
    
    target_link_libraries(cmpth INTERFACE massivethreads)
endif()



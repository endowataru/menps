cmake_minimum_required(VERSION 3.1)

project(cmpth CXX)

option(CMPTH_ENABLE_MTH "Build MassiveThreads" ON)

add_library(cmpth INTERFACE)
target_include_directories(cmpth INTERFACE ./include)
install(DIRECTORY include/ DESTINATION include)

set(CMPTH_GLOBAL_CFLAGS -ftls-model=initial-exec)
# CMPTH_GLOBAL_CFLAGS will be used in massivethread.cmkae

add_library(cmpth-options INTERFACE)

target_compile_options(cmpth-options INTERFACE
    -Wall -Wextra -pedantic ${CMPTH_GLOBAL_CFLAGS}
)


add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE
    ./external/doctest/doctest
)


add_executable(cmpth-mth-test
    ./example/mth-test.cpp
)
target_link_libraries(cmpth-mth-test
    cmpth
    cmpth-options
)
set_target_properties(cmpth-mth-test PROPERTIES
    CXX_STANDARD 11
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON)


add_executable(cmpth_test_smth
    ./tests/test_smth.cpp
)
target_link_libraries(cmpth_test_smth
    cmpth
    cmpth-options
    doctest
)
set_target_properties(cmpth_test_smth PROPERTIES
    CXX_STANDARD 11
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON)

enable_testing()

add_test(
    NAME cmpth_test_smth
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_sweep.sh
        CMPTH_NUM_WORKERS $<TARGET_FILE:cmpth_test_smth>
)


if (${CMPTH_ENABLE_MTH})
    include(./cmake/massivethreads.cmake)
    
    target_link_libraries(cmpth INTERFACE massivethreads)
    
    add_executable(cmpth_test_mth
        ./tests/test_mth.cpp
    )
    target_link_libraries(cmpth_test_mth
        cmpth
        cmpth-options
        doctest
    )
    set_target_properties(cmpth_test_mth PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON)
    
    add_test(
        NAME cmpth_test_mth
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_sweep.sh
            MYTH_NUM_WORKERS $<TARGET_FILE:cmpth_test_mth>
    )
endif()



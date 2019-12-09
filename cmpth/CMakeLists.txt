cmake_minimum_required(VERSION 3.1)

project(cmpth CXX)

option(CMPTH_ENABLE_MTH "Build MassiveThreads" ON)
option(CMPTH_ENABLE_MLOG "Use MassiveLogger" OFF)
option(CMPTH_ENABLE_ABT "Build Argobots" OFF)

add_library(cmpth INTERFACE)
target_include_directories(cmpth INTERFACE ./include)
install(DIRECTORY include/ DESTINATION include)

set(CMPTH_GLOBAL_CFLAGS -ftls-model=initial-exec)
# CMPTH_GLOBAL_CFLAGS will be used in massivethreads.cmake and argobots.cmake

add_library(cmpth-options INTERFACE)

target_compile_options(cmpth-options INTERFACE
    -Wall -Wextra -pedantic ${CMPTH_GLOBAL_CFLAGS}
)

function(cmpth_require_cxx11 target)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON)
endfunction()


add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE
    ./external/doctest/doctest
)


add_executable(cmpth_test_sct
    ./tests/test_sct.cpp
)
target_link_libraries(cmpth_test_sct
    cmpth
    cmpth-options
    doctest
)
cmpth_require_cxx11(cmpth_test_sct)

enable_testing()

add_test(
    NAME cmpth_test_sct
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_sweep.sh
        CMPTH_NUM_WORKERS $<TARGET_FILE:cmpth_test_sct>
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
    cmpth_require_cxx11(cmpth_test_mth)
    
    add_test(
        NAME cmpth_test_mth
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_sweep.sh
            MYTH_NUM_WORKERS $<TARGET_FILE:cmpth_test_mth>
    )
    
    
    add_executable(cmpth_test_ctmth
        ./tests/test_ctmth.cpp
    )
    target_link_libraries(cmpth_test_ctmth
        cmpth
        cmpth-options
        doctest
    )
    cmpth_require_cxx11(cmpth_test_ctmth)
    
    add_test(
        NAME cmpth_test_ctmth
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_sweep.sh
            MYTH_NUM_WORKERS $<TARGET_FILE:cmpth_test_ctmth>
    )
endif()

if (${CMPTH_ENABLE_MLOG})
    add_library(massivelogger INTERFACE)
    target_include_directories(massivelogger INTERFACE ./external/massivelogger/include)
    target_link_libraries(cmpth INTERFACE massivelogger)
endif()

if (${CMPTH_ENABLE_ABT})
    include(./cmake/argobots.cmake)
    
    add_executable(cmpth-abt-test
        ./example/abt/abt-test.cpp
    )
    target_link_libraries(cmpth-abt-test
        cmpth
        cmpth-options
        argobots
    )
    cmpth_require_cxx11(cmpth-abt-test)
endif()

add_library(cmpth-all INTERFACE)
target_link_libraries(cmpth-all INTERFACE
    cmpth
    $<$<BOOL:${CMPTH_ENABLE_ABT}>:argobots>
)
target_compile_definitions(cmpth-all INTERFACE
    $<$<BOOL:${CMPTH_ENABLE_ABT}>:CMPTH_ENABLE_ABT=1>
)

if (${CMPTH_ENABLE_MTH}) # TODO
    file(GLOB example_files example/*.cpp)
    foreach(example_path IN LISTS example_files)
        get_filename_component(example_name ${example_path} NAME_WE)
        add_executable(cmpth-${example_name}
            ${example_path}
        )
        target_link_libraries(cmpth-${example_name}
            cmpth-all
            cmpth-options
            doctest
        )
        cmpth_require_cxx11(cmpth-${example_name})
    endforeach()
endif()



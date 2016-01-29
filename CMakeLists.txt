cmake_minimum_required(VERSION 3.0)

project(mgbase)

add_library(mgbase-header INTERFACE)
target_include_directories(mgbase-header
    INTERFACE ./include
    INTERFACE ./external
)
target_compile_definitions(mgbase-header
    INTERFACE "$<$<CONFIG:Debug>:MGBASE_DEBUG>"
    INTERFACE "$<$<CONFIG:RelWithDebInfo>:MGBASE_DEBUG>"
)

set(error_limit 3)

add_library(mgbase-options INTERFACE)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Fujitsu")
    target_compile_options(mgbase-options INTERFACE
        -Xg
        -D GTEST_HAS_TR1_TUPLE=0
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-Kfast>
        $<$<CONFIG:RelWithDebInfo>:-g -Kfast>
    )
else()
    target_compile_options(mgbase-options INTERFACE
        -Wall
        #$<$<COMPILE_LANGUAGE:CXX>:-std=c++03> # TODO: COMPILE_LANGUAGE doesn't work on old CMake versions
        -pedantic
        -Wextra
        -Wcast-qual
        -Wctor-dtor-privacy
        -Wdisabled-optimization
        -Wformat=2
        -Winit-self
        -Wmissing-declarations
        -Wmissing-include-dirs
        -Woverloaded-virtual
        -Wredundant-decls
        -Wshadow
        -Wsign-promo
        -Wstrict-overflow=5
        -Wswitch-default
        -Wno-variadic-macros
        $<$<CXX_COMPILER_ID:Clang>:-Wno-c++11-long-long> # for mpi.h
        $<$<CXX_COMPILER_ID:GNU>:-Wno-long-long> # for mpi.h
        $<$<CXX_COMPILER_ID:Clang>:-ferror-limit=${error_limit}>
        $<$<CXX_COMPILER_ID:GNU>:-fmax-errors=${error_limit}>
    )
endif()


add_library(mgbase
    ./external/cppformat/format.cc
)
target_link_libraries(mgbase mgbase-header)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Fujitsu")
    target_compile_options(mgbase
        PRIVATE
        -Xg
        -include ${CMAKE_CURRENT_SOURCE_DIR}/include/mgbase/external/cppformat.hpp
    )
endif()

add_library(googletest
    ./external/googletest/googletest/src/gtest-all.cc
)
target_include_directories(googletest
    PUBLIC ./external/googletest/googletest
    PUBLIC ./external/googletest/googletest/include
)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Fujitsu")
    target_sources(googletest
        PRIVATE ./src/workaround/wcscasecmp.cpp
    )
    target_compile_options(googletest
        PRIVATE
        -Xg
        -include ${CMAKE_CURRENT_SOURCE_DIR}/src/workaround/wcscasecmp.h
        -D GTEST_HAS_TR1_TUPLE=0
    )
endif()

add_executable(mgbase-unittest
    unittest/unittest.cpp
    unittest/deferred.cpp
    unittest/thread.cpp
    unittest/bound_function.cpp
    unittest/spinlock.cpp
    unittest/mpsc_circular_buffer.cpp
    unittest/condition_variable.cpp
    unittest/optional.cpp
)

target_link_libraries(mgbase-unittest mgbase mgbase-options googletest)


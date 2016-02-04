cmake_minimum_required(VERSION 3.0)

project(mgbase)

option(MGBASE_PROFILER_USE_GPROF "Use Gprof" OFF)
option(MGBASE_PROFILER_USE_GPERFTOOLS "Use Gperftools" OFF)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/mgbase/config.h)

message(STATUS "Generate mgbase/config.h ...")
message(STATUS "MGBASE_PROFILER_USE_GPROF=${MGBASE_PROFILER_USE_GPROF}")
message(STATUS "MGBASE_PROFILER_USE_GPERFTOOLS=${MGBASE_PROFILER_USE_GPERFTOOLS}")

add_library(mgbase-header INTERFACE)
target_include_directories(mgbase-header
    INTERFACE ./include
    INTERFACE ./external
    INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
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
        $<$<CXX_COMPILER_ID:Clang>:-Weverything>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-global-constructors>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-exit-time-destructors>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-padded> # TODO: alignment issues
        $<$<CXX_COMPILER_ID:Clang>:-Wno-disabled-macro-expansion> # Problem of stdbool.h
        $<$<CXX_COMPILER_ID:Clang>:-Wno-used-but-marked-unused> # e.g. mgbase::try_to_lock
        $<$<CXX_COMPILER_ID:Clang>:-Wno-weak-vtables> # couldn't find a correct way to suppress this warning
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
    if(MGBASE_PROFILER_USE_GPROF)
    target_compile_options(mgbase-options INTERFACE
        $<$<CXX_COMPILER_ID:GNU>:-pg>
    )
    target_link_libraries(mgbase-options INTERFACE "-pg")
    endif()
endif()


add_library(mgbase
    ./external/cppformat/format.cc
)
target_link_libraries(mgbase mgbase-header)

if (MGBASE_PROFILER_USE_GPERFTOOLS)
    target_link_libraries(mgbase profiler)
endif()

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



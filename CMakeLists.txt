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
    INTERFACE ./external/cppformat
    INTERFACE ./external/cmdline
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
        --display_error_number
        --diag_suppress 111,161,236,1290,1406
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-Kfast -O3 -x50>
        $<$<CONFIG:RelWithDebInfo>:-g -Kfast>
    )
    # warning #111-D: statement is unreachable
    #   (cppformat)
    # warning #161-D: unrecognized #pragma
    #   (cppformat, googletest)
    # warning #236-D: controlling expression is constant
    #   (googletest)
    # warning #1290-D: non-POD class type passed through ellipsis
    #   (googletest)
    # warning #1406-D: support for trigraphs is disabled
    #   (googletest)
else()
    target_compile_options(mgbase-options INTERFACE
        -Wall
        #$<$<COMPILE_LANGUAGE:CXX>:-std=c++03> # TODO: COMPILE_LANGUAGE doesn't work on old CMake versions
        -DGTEST_HAS_TR1_TUPLE=0
        -pedantic
        -Wextra
        -Wcast-qual
        -Wdisabled-optimization
        -Wformat=2
        -Winit-self
        -Wmissing-declarations
        -Wmissing-include-dirs
        -Woverloaded-virtual
        -Wsign-promo
        -Wno-variadic-macros
    )
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options(mgbase-options INTERFACE
            # Enable -Weverything
            -Weverything
            # Disable some warnings of -Weverything
            -Wno-global-constructors
            -Wno-exit-time-destructors
            -Wno-padded # TODO: alignment issues
            -Wno-disabled-macro-expansion # Problem of stdbool.h
            -Wno-used-but-marked-unused # e.g. mgbase::try_to_lock
            -Wno-weak-vtables # couldn't find a correct way to suppress this warning
            # Other options
            -Wshadow
            -Wno-c++11-long-long # for mpi.h
            -ferror-limit=${error_limit}
            -Wstrict-overflow=5
            -Wctor-dtor-privacy
            -Wswitch-default
            -Wredundant-decls
        )
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GCC")
        target_compile_options(mgbase-options INTERFACE
            -Wshadow
            -Wno-long-long
            -fmax-errors=${error_limit}
            -Wstrict-overflow=5
            -Wctor-dtor-privacy
            -Wswitch-default
            -Wredundant-decls
        )
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
        target_compile_options(mgbase-options INTERFACE
            -Wno-long-long
            -fmax-errors=${error_limit}
            -Wno-shadow # ICC cannot disable this warning in template classes/functions
        )
    endif()
    
    if(MGBASE_PROFILER_USE_GPROF)
    target_compile_options(mgbase-options INTERFACE
        $<$<CXX_COMPILER_ID:GNU>:-pg>
    )
    target_link_libraries(mgbase-options INTERFACE "-pg")
    endif()
endif()


add_library(mgbase
    ./external/cppformat/cppformat/format.cc
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
target_compile_options(googletest
    PRIVATE
    -w # Suppress ALL warnings
)
target_include_directories(googletest
    PUBLIC ./external/googletest/googletest
    PUBLIC ./external/googletest/googletest/include
)
target_link_libraries(googletest
    PRIVATE mgbase-options
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
    unittest/callback_function.cpp
    unittest/bound_function.cpp
    unittest/spinlock.cpp
    unittest/mpsc_circular_buffer.cpp
    unittest/condition_variable.cpp
    unittest/optional.cpp
)

target_link_libraries(mgbase-unittest mgbase mgbase-options googletest)



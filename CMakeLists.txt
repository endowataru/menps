cmake_minimum_required(VERSION 3.0)

project(mgbase)

find_package(Threads)

INCLUDE (CheckCXXCompilerFlag)

option(MGBASE_PROFILER_USE_GPROF "Use Gprof" OFF)
option(MGBASE_PROFILER_USE_GPERFTOOLS "Use Gperftools" OFF)
option(MGBASE_ATOMIC_USE_STANDARD "Use <atomic>" OFF)

set(MGBASE_CXX_STANDARD "0x" CACHE STRING "C++ standard version")

add_library(mgbase-header INTERFACE)
target_include_directories(mgbase-header
    INTERFACE ./include
    INTERFACE ./external/fmt
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
    #   (fmt)
    # warning #161-D: unrecognized #pragma
    #   (fmt, googletest)
    # warning #236-D: controlling expression is constant
    #   (googletest)
    # warning #1290-D: non-POD class type passed through ellipsis
    #   (googletest)
    # warning #1406-D: support for trigraphs is disabled
    #   (googletest)
else()
    target_compile_options(mgbase-options INTERFACE
        #$<$<COMPILE_LANGUAGE:CXX>:-std=c++03> # TODO: COMPILE_LANGUAGE doesn't work on old CMake versions
        -DGTEST_HAS_TR1_TUPLE=0
        -Wno-variadic-macros
    )
    if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
        target_compile_options(mgbase-options INTERFACE
            # C++ Standard
            $<$<NOT:$<STREQUAL:"${MGBASE_CXX_STANDARD}","">>:-std=c++${MGBASE_CXX_STANDARD}>
            
            # Optimizations
            $<$<CONFIG:Debug>:-O0>
            
            # Enable -Weverything
            -Weverything
            
            # Disable some warnings of -Weverything
            -Wno-global-constructors
            -Wno-exit-time-destructors
            -Wno-padded # TODO: alignment issues
            -Wno-disabled-macro-expansion # Problem of stdbool.h
            -Wno-used-but-marked-unused # e.g. mgbase::try_to_lock
            -Wno-weak-vtables # couldn't find a correct way to suppress this warning
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            
            # Other options
            -Wno-c++11-long-long # TODO
            -ferror-limit=${error_limit}
        )
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        # Get the version
        execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                OUTPUT_VARIABLE MGBASE_COMPILER_GCC_VERSION)
        string(REGEX MATCHALL "[0-9]+" MGBASE_COMPILER_GCC_VERSION_COMPONENTS ${MGBASE_COMPILER_GCC_VERSION})
        list(GET MGBASE_COMPILER_GCC_VERSION_COMPONENTS 0 MGBASE_COMPILER_GCC_VERSION_MAJOR)
        list(GET MGBASE_COMPILER_GCC_VERSION_COMPONENTS 1 MGBASE_COMPILER_GCC_VERSION_MINOR)
        message(STATUS "MGBASE_COMPILER_GCC_VERSION=${MGBASE_COMPILER_GCC_VERSION}")
        message(STATUS "MGBASE_COMPILER_GCC_VERSION_MAJOR=${MGBASE_COMPILER_GCC_VERSION_MAJOR}")
        message(STATUS "MGBASE_COMPILER_GCC_VERSION_MINOR=${MGBASE_COMPILER_GCC_VERSION_MINOR}")
        
        CHECK_CXX_COMPILER_FLAG("-Wc++14-compat" MGBASE_GCC_SUPPORTS_WCXX14_COMPAT)
        CHECK_CXX_COMPILER_FLAG("-Wconditionally-supported" MGBASE_GCC_SUPPORTS_WCONDITINALLY_SUPPORTED)
        CHECK_CXX_COMPILER_FLAG("-Wdate-time" MGBASE_GCC_SUPPORTS_WDATE_TIME)
        CHECK_CXX_COMPILER_FLAG("-Wdelete-non-virtual-dtor" MGBASE_GCC_SUPPORTS_WDELETE_NON_VIRTUAL_DTOR)
        CHECK_CXX_COMPILER_FLAG("-Wdouble-promotion" MGBASE_GCC_SUPPORTS_WDOUBLE_PROMOTION)
        CHECK_CXX_COMPILER_FLAG("-Wformat-signedness" MGBASE_GCC_SUPPORTS_WFORMAT_SIGNEDNESS)
        CHECK_CXX_COMPILER_FLAG("-Wnoexcept" MGBASE_GCC_SUPPORTS_WNOEXCEPT)
        CHECK_CXX_COMPILER_FLAG("-Wsuggest-attribute=format" MGBASE_GCC_SUPPORTS_WSUGGEST_ATTRIBUTE)
        CHECK_CXX_COMPILER_FLAG("-Wsuggest-override" MGBASE_GCC_SUPPORTS_WSUGGEST_OVERRIDE)
        CHECK_CXX_COMPILER_FLAG("-Wuseless-cast" MGBASE_GCC_SUPPORTS_WUSELESS_CAST)
        CHECK_CXX_COMPILER_FLAG("-Wzero-as-null-pointer-constant" MGBASE_GCC_SUPPORTS_WZERO_AS_NULL_POINTER_CONSTANT)
        
        target_compile_options(mgbase-options INTERFACE
            # C++ Standard
            $<$<NOT:$<STREQUAL:"${MGBASE_CXX_STANDARD}","">>:-std=c++${MGBASE_CXX_STANDARD}>
            
            # Optimizations
            $<$<CONFIG:Debug>:-O0>
            
            # Meta warnings
            -Wall
            -Wextra
            -pedantic
            
            # Disable warnings
            -Wno-long-long # TODO
            
            # Generated by this command:
            #   $ gcc -Q --help=warning -Wall -pedantic -Wextra | grep disabled | cut -f 1
            #-Wabi # Seems useless
            #-Wabi-tag # Seems useless
            -Wc++0x-compat
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WCXX14_COMPAT}>:-Wc++14-compat>
            -Wcast-qual
            #-Wchkp # Undocumented warning option
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WCONDITINALLY_SUPPORTED}>:-Wconditionally-supported>
            -Wconversion
            $<$<VERSION_GREATER:${MGBASE_COMPILER_GCC_VERSION},4.5>:-Wctor-dtor-privacy> # #pragma GCC diagnostic is required
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WDATE_TIME}>:-Wdate-time>
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WDELETE_NON_VIRTUAL_DTOR}>:-Wdelete-non-virtual-dtor>
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WDOUBLE_PROMOTION}>:-Wdouble-promotion>
            #-Weffc++ # TODO: Too strict?
            -Wfloat-equal
            -Wformat-nonliteral
            -Wformat-security
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WFORMAT_SIGNEDNESS}>:-Wformat-signedness>
            -Wformat-y2k
            -Winit-self
            -Winvalid-pch
            -Wlogical-op
            -Wmissing-declarations
            -Wmissing-include-dirs
            -Wmultichar
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WNOEXCEPT}>:-Wnoexcept>
            -Wnon-virtual-dtor
            $<$<VERSION_GREATER:${MGBASE_COMPILER_GCC_VERSION},4.5>:-Wold-style-cast> # #pragma GCC diagnostic is required
            -Woverloaded-virtual
            -Wredundant-decls
            -Wreorder
            -Wsign-promo
            -Wstrict-null-sentinel
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WSUGGEST_ATTRIBUTE}>:-Wsuggest-attribute=format>
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WSUGGEST_OVERRIDE}>:-Wsuggest-override>
            -Wswitch-default
            -Wswitch-enum
            -Wsynth
            #-Wsystem-headers # Disable warnings from system headers
            -Wundef
            -Wunused-macros
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WUSELESS_CAST}>:-Wuseless-cast>
            -Wwrite-strings
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WZERO_AS_NULL_POINTER_CONSTANT}>:-Wzero-as-null-pointer-constant>
            
            # Other warnings
            -Wstrict-overflow=5
            
            # Disabled on old versions
            $<$<VERSION_LESS:${MGBASE_COMPILER_GCC_VERSION},4.5>:-Wno-logical-op>
        )
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
        target_compile_options(mgbase-options INTERFACE
            # C++ Standard
            $<$<NOT:$<STREQUAL:"${MGBASE_CXX_STANDARD}","">>:-std=c++${MGBASE_CXX_STANDARD}>
            
            # Optimizations
            $<$<CONFIG:Debug>:-O0>
            
            # Enable -Wall
            -Wall
            -Wextra
            -pedantic
            
            # Disable some warnings
            -Wno-long-long
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

CHECK_CXX_SOURCE_COMPILES(
"
__attribute__((always_inline))
void f() { }

int main() { return 0; }
" MGBASE_COMPILER_SUPPORTS_ALWAYS_INLINE)

CHECK_CXX_SOURCE_COMPILES(
"
void f() { __builtin_unreachable(); }

int main() { return 0; }
" MGBASE_COMPILER_SUPPORTS_BUILTIN_UNREACHABLE
)

add_library(mgbase
    ./external/fmt/fmt/format.cc
    ./external/fmt/fmt/ostream.cc
    )

target_link_libraries(mgbase mgbase-header 
    pthread # TODO: Use ${CMAKE_THREAD_LIBS_INIT} ?
    )

if (MGBASE_PROFILER_USE_GPERFTOOLS)
    target_link_libraries(mgbase profiler)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Fujitsu")
    target_compile_options(mgbase
        PRIVATE
        -Xg
        -include ${CMAKE_CURRENT_SOURCE_DIR}/include/mgbase/external/fmt.hpp
    )
endif()

# Google Test

add_library(googletest
    ./external/googletest/googletest/src/gtest-all.cc
)
target_compile_options(googletest
    PRIVATE
    -w # Suppress ALL warnings
    -DGTEST_HAS_TR1_TUPLE=0
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
    )
endif()

# Unit tests

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
    unittest/move.cpp
)

target_link_libraries(mgbase-unittest mgbase mgbase-options googletest)

message(STATUS "Generate mgbase/config.h ...")
message(STATUS "MGBASE_PROFILER_USE_GPROF=${MGBASE_PROFILER_USE_GPROF}")
message(STATUS "MGBASE_PROFILER_USE_GPERFTOOLS=${MGBASE_PROFILER_USE_GPERFTOOLS}")
message(STATUS "MGBASE_CXX_STANDARD=${MGBASE_CXX_STANDARD}")
message(STATUS "MGBASE_ATOMIC_USE_STANDARD=${MGBASE_ATOMIC_USE_STANDARD}")

# Generate config.h

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/mgbase/config.h)


# MassiveThreads

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    if (${MGBASE_ENABLE_MASSIVETHREADS})
        include(cmake/massivethreads.cmake)
    endif()
endif()


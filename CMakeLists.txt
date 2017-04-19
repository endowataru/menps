cmake_minimum_required(VERSION 3.0)

project(mgbase CXX ASM)

INCLUDE(CheckCXXCompilerFlag)

# Threading is required.
# This package adds CMAKE_THREAD_LIBS_INIT, which is used later.
find_package(Threads)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

# BUILD_SHARED_LIBS is a common flag in CMake that decides shared/static libs.
# I don't know whether it's correct to use it as an option.
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(MGBASE_PROFILER_USE_GPROF "Use Gprof" OFF)
option(MGBASE_PROFILER_USE_GPERFTOOLS "Use Gperftools" OFF)
option(MGBASE_ATOMIC_USE_STANDARD "Use <atomic>" OFF)
option(MGBASE_LOGGER_USE_LOCK "Use lock in logger" ON)

set(MGBASE_CXX_STANDARD "0x" CACHE STRING "C++ standard version")

set(error_limit 3)

# mgbase-options : INTERFACE target to set options individually

add_library(mgbase-options INTERFACE)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Fujitsu")
    target_compile_options(mgbase-options INTERFACE
        -Xg
        -D GTEST_HAS_TR1_TUPLE=0
        --display_error_number
        --diag_suppress 111,161,236,1290,1406
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-Kfast -O3 -x50>
        $<$<CONFIG:OptDebug>:-g -Kfast>
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
        # C++ Standard
        $<$<COMPILE_LANGUAGE:CXX>:
            $<$<NOT:$<STREQUAL:"${MGBASE_CXX_STANDARD}","">>:-std=c++${MGBASE_CXX_STANDARD}>
        >
        # C Standard
        $<$<COMPILE_LANGUAGE:C>:-std=c99>
        
        #$<$<COMPILE_LANGUAGE:CXX>:-std=c++03> # TODO: COMPILE_LANGUAGE doesn't work on old CMake versions
        -DGTEST_HAS_TR1_TUPLE=0
        -Wno-variadic-macros
    )
    if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
        target_compile_options(mgbase-options INTERFACE
            # Optimizations
            $<$<CONFIG:Debug>:-O0>
            $<$<CONFIG:OptDebug>:-O3 -DNDEBUG>
            
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
            -Wno-abstract-vbase-init # Not approved by C++ standard
            
            # Other options
            -Wno-c++11-long-long # TODO
            -ferror-limit=${error_limit}
            
            # Bug 25546 - Bad diagnostic when using -Wunused-macros and building .S files
            # https://llvm.org/bugs/show_bug.cgi?id=25546
            $<$<COMPILE_LANGUAGE:ASM>:-Wno-unused-macros>
        )
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        # Get the version
        set(MGBASE_COMPILER_GCC_VERSION ${CMAKE_CXX_COMPILER_VERSION})
        message(STATUS "MGBASE_COMPILER_GCC_VERSION=${MGBASE_COMPILER_GCC_VERSION}")
        
        CHECK_CXX_COMPILER_FLAG("-Wc++14-compat" MGBASE_GCC_SUPPORTS_WCXX14_COMPAT)
        CHECK_CXX_COMPILER_FLAG("-Wconditionally-supported" MGBASE_GCC_SUPPORTS_WCONDITINALLY_SUPPORTED)
        CHECK_CXX_COMPILER_FLAG("-Wdelete-non-virtual-dtor" MGBASE_GCC_SUPPORTS_WDELETE_NON_VIRTUAL_DTOR)
        CHECK_CXX_COMPILER_FLAG("-Wdouble-promotion" MGBASE_GCC_SUPPORTS_WDOUBLE_PROMOTION)
        CHECK_CXX_COMPILER_FLAG("-Wformat-signedness" MGBASE_GCC_SUPPORTS_WFORMAT_SIGNEDNESS)
        CHECK_CXX_COMPILER_FLAG("-Wnoexcept" MGBASE_GCC_SUPPORTS_WNOEXCEPT)
        CHECK_CXX_COMPILER_FLAG("-Wsuggest-attribute=format" MGBASE_GCC_SUPPORTS_WSUGGEST_ATTRIBUTE)
        CHECK_CXX_COMPILER_FLAG("-Wsuggest-override" MGBASE_GCC_SUPPORTS_WSUGGEST_OVERRIDE)
        #CHECK_CXX_COMPILER_FLAG("-Wuseless-cast" MGBASE_GCC_SUPPORTS_WUSELESS_CAST)
        CHECK_CXX_COMPILER_FLAG("-Wzero-as-null-pointer-constant" MGBASE_GCC_SUPPORTS_WZERO_AS_NULL_POINTER_CONSTANT)
        CHECK_CXX_COMPILER_FLAG("-fmax-errors=1" MGBASE_GCC_SUPPORTS_FMAX_ERRORS)
        
        target_compile_options(mgbase-options INTERFACE
            # Optimizations
            $<$<CONFIG:Debug>:-O0>
            $<$<CONFIG:OptDebug>:-O3 -DNDEBUG>
            
            # Disable SSP.
            -fno-stack-protector
                # TODO : this is only needed for mgth
            
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
            #-Wc++0x-compat
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WCXX14_COMPAT}>:-Wc++14-compat>
            -Wcast-qual
            #-Wchkp # Undocumented warning option
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WCONDITINALLY_SUPPORTED}>:-Wconditionally-supported>
            -Wconversion
            $<$<VERSION_GREATER:${MGBASE_COMPILER_GCC_VERSION},4.5>:-Wctor-dtor-privacy> # #pragma GCC diagnostic is required
            #-Wdate-time # Seems useless
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WDELETE_NON_VIRTUAL_DTOR}>:-Wdelete-non-virtual-dtor>
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WDOUBLE_PROMOTION}>:-Wdouble-promotion>
            #-Weffc++ # Out-dated warning
            -Wfloat-equal
            -Wformat-nonliteral
            -Wformat-security
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WFORMAT_SIGNEDNESS}>:-Wformat-signedness>
            #-Wformat-y2k # Not important
            -Winit-self
            #-Winvalid-pch # TODO
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
            #$<$<BOOL:${MGBASE_GCC_SUPPORTS_WUSELESS_CAST}>:-Wuseless-cast>
            -Wwrite-strings
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_WZERO_AS_NULL_POINTER_CONSTANT}>:-Wzero-as-null-pointer-constant>
            
            # Other warnings
            -Wstrict-overflow=5
            
            # Disabled on old versions
            $<$<VERSION_LESS:${MGBASE_COMPILER_GCC_VERSION},4.5>:-Wno-logical-op>
            
            # Prevent too many errors
            #-Wfatal-errors
            $<$<BOOL:${MGBASE_GCC_SUPPORTS_FMAX_ERRORS}>:-fmax-errors=${error_limit}>
        )
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
        target_compile_options(mgbase-options INTERFACE
            # Optimizations
            $<$<CONFIG:Debug>:-O0>
            $<$<CONFIG:OptDebug>:-O3 -DNDEBUG>
            
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

CHECK_CXX_SOURCE_COMPILES("
__attribute__((always_inline))
void f() { }

int main() { return 0; }
" MGBASE_COMPILER_SUPPORTS_ALWAYS_INLINE)

CHECK_CXX_SOURCE_COMPILES("
void f() { __builtin_unreachable(); }

int main() { return 0; }
" MGBASE_COMPILER_SUPPORTS_BUILTIN_UNREACHABLE)

# To check C++0x features, we need this flag
set(CMAKE_REQUIRED_FLAGS "-std=c++0x")

CHECK_CXX_SOURCE_COMPILES("
void f() noexcept { }
int main() { return 0; }
" MGBASE_CXX11_NOEXCEPT_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
int main() { int* x = nullptr; return 0; }
" MGBASE_CXX11_NULLPTR_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
#include <cstddef>
int main() { std::nullptr_t x = nullptr; return 0; }
" MGBASE_CXX11_NULLPTR_T_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
#include <memory>
int main() { std::unique_ptr<int> p(new int); return 0; }
" MGBASE_CXX11_UNIQUE_PTR_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
struct A {
    A(A&&) = default;
};
int main() { return 0; }
" MGBASE_CXX11_MOVE_CONSTRUCTOR_DEFAULT_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
struct A {
    A(A&&) = default;
};
int main() { return 0; }
" MGBASE_CXX11_MOVE_ASSIGNMENT_DEFAULT_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES(
"
int main() {
    int a[1];
    for (int x : a) { }
    return 0;
}
" MGBASE_CXX11_RANGE_BASED_FOR_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
struct X { void f() &; void g() &&; };
int main() { return 0; }
" MGBASE_CXX11_REF_QUALIFIERS_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
struct X { explicit operator bool(); };
int main() { return 0; }
" MGBASE_CXX11_EXPLICIT_CONVERSION_OPERATORS_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
template <typename T> T r(T x) { return x; }

template <typename F, typename... T>
inline auto f(F&& f, T&&... x) -> decltype(r(f)(x...)) { return f(x...); }

int g(int);

decltype(f(&g, 123)) x;

int main() { return 0; }
" MGBASE_CXX_SFINAE_EXPR)

CHECK_CXX_SOURCE_COMPILES("
struct A { A() noexcept = default; };
int main() { return 0; }
" MGBASE_CXX11_DEFAULT_FUNCTION_NOEXCEPT)

CHECK_CXX_SOURCE_COMPILES("
thread_local int x;
int main() { return 0; }
" MGBASE_CXX11_THREAD_LOCAL_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
int main() { alignof(int); return 0; }
" MGBASE_CXX11_ALIGNOF_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
struct A {
    A() { }
    A(const A&) = delete;
    A(A&&) { }
    A& operator = (const A&) = delete;
    A& operator = (A&) { return *this; }
};
A f(A a) { return a; };
int main() { return 0; }
" MGBASE_CXX11_RETURN_MOVE_ONLY_ARGUMENT_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
#include <iterator>
int main() { int a[1]; std::begin(a); return 0; }
" MGBASE_CXX11_STD_BEGIN_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
#include <iterator>
int main() { int a[1]; std::end(a); return 0; }
" MGBASE_CXX11_STD_END_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
#include <memory>
int main() { int x; int* p = std::addressof(x); return 0; }
" MGBASE_CXX11_ADDRESSOF_SUPPORTED)

message(STATUS "Generate mgbase/config.h ...")
message(STATUS "BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}")
message(STATUS "MGBASE_PROFILER_USE_GPROF=${MGBASE_PROFILER_USE_GPROF}")
message(STATUS "MGBASE_PROFILER_USE_GPERFTOOLS=${MGBASE_PROFILER_USE_GPERFTOOLS}")
message(STATUS "MGBASE_CXX_STANDARD=${MGBASE_CXX_STANDARD}")
message(STATUS "MGBASE_ATOMIC_USE_STANDARD=${MGBASE_ATOMIC_USE_STANDARD}")
message(STATUS "MGBASE_ULT_DISABLE_YIELD=${MGBASE_ULT_DISABLE_YIELD}")
message(STATUS "MGBASE_DISABLE_STANDARD_UNIQUE_PTR=${MGBASE_DISABLE_STANDARD_UNIQUE_PTR}")
message(STATUS "MGBASE_LOGGER_USE_LOCK=${MGBASE_LOGGER_USE_LOCK}")

# Generate config.h

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/mgbase/config.h)


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

# mgbase : SHARED/STATIC target

add_library(mgbase
    ./external/fmt/fmt/format.cc
    ./external/fmt/fmt/ostream.cc
    ./src/malloc.cpp
)
target_include_directories(mgbase
    PUBLIC ./include
    PUBLIC ./external/fmt
    PUBLIC ./external/cmdline
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include # only for mgbase/config.h
)
target_compile_definitions(mgbase
    PUBLIC "$<$<CONFIG:Debug>:MGBASE_DEBUG>"
)
target_link_libraries(mgbase
    PUBLIC ${CMAKE_THREAD_LIBS_INIT} # Add "-lpthread".
    PRIVATE mgbase-options
)
install(TARGETS mgbase
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

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
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include # only for mgbase/config.h
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
    )
endif()

# Unit tests

add_executable(mgbase-unittest
    unittest/mpsc_bounded_queue.cpp
    unittest/spmc_bounded_queue.cpp
    unittest/spsc_bounded_queue.cpp
    unittest/mpsc_locked_bounded_queue.cpp
    unittest/intrusive.cpp
    unittest/index_list.cpp
    unittest/bind.cpp
    unittest/tuple.cpp
    unittest/callback.cpp
    unittest/unittest.cpp
    unittest/deferred.cpp
    unittest/thread.cpp
    unittest/callback_function.cpp
    unittest/spinlock.cpp
    unittest/condition_variable.cpp
    #unittest/optional.cpp
)

target_link_libraries(mgbase-unittest
    mgbase
    mgbase-options
    googletest
)


# MassiveThreads

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    if (${MGBASE_ENABLE_MASSIVETHREADS})
        include(cmake/massivethreads.cmake)
    endif()
endif()


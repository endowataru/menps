cmake_minimum_required(VERSION 3.0)

project(mefdn CXX C ASM)

INCLUDE(CheckCXXCompilerFlag)

# Threading is required.
# This package adds CMAKE_THREAD_LIBS_INIT, which is used later.
find_package(Threads)

find_package(MPI REQUIRED)
#message(STATUS "Run: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")
#message(STATUS "MPI_GUESS_LIBRARY_NAME=${MPI_GUESS_LIBRARY_NAME}")

if(NOT TARGET MPI::MPI_CXX)
    add_library(MPI::MPI_CXX IMPORTED INTERFACE)
    
    set_property(TARGET MPI::MPI_CXX
                 PROPERTY INTERFACE_COMPILE_OPTIONS ${MPI_CXX_COMPILE_FLAGS})
    set_property(TARGET MPI::MPI_CXX
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${MPI_CXX_INCLUDE_PATH}")
    set_property(TARGET MPI::MPI_CXX
                 PROPERTY INTERFACE_LINK_LIBRARIES ${MPI_CXX_LINK_FLAGS} ${MPI_CXX_LIBRARIES})
endif()

if (NOT DEFINED MPIEXEC_EXECUTABLE)
    set(MPIEXEC_EXECUTABLE ${MPIEXEC} PARENT_SCOPE)
endif()

execute_process(
    COMMAND ${MPIEXEC_EXECUTABLE} --allow-run-as-root ${MPIEXEC_PREFLAGS} echo a ${MPIEXEC_POSTFLAGS}
    RESULT_VARIABLE MEFDN_MPIEXEC_HAS_ALLOW_RUN_AS_ROOT_RESULT
    OUTPUT_QUIET ERROR_QUIET
)

if (MEFDN_MPIEXEC_HAS_ALLOW_RUN_AS_ROOT_RESULT)
    set(MEFDN_MPIEXEC_PREFLAGS "${MPIEXEC_PREFLAGS}" PARENT_SCOPE)
else()
    set(MEFDN_MPIEXEC_PREFLAGS "${MPIEXEC_PREFLAGS} --allow-run-as-root --oversubscribe --bind-to none" PARENT_SCOPE)
endif()

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

# BUILD_SHARED_LIBS is a common flag in CMake that decides shared/static libs.
# I don't know whether it's correct to use it as an option.
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(MEFDN_ENABLE_ASSERT "Enable assertion explicitly" OFF)
option(MEFDN_ENABLE_LOG "Enable logging explicitly" OFF)
option(MEFDN_PROFILER_USE_GPROF "Use Gprof" OFF)
option(MEFDN_LOGGER_USE_LOCK "Use lock in logger" ON)
option(MEFDN_ENABLE_EXPONENTIAL_BACKOFF "Use exponential backoff" OFF)
option(MEFDN_DISABLE_ALIGN_CHECK_SIZE "Disable size check in mefdn::align_*_nocheck" ON)
option(MEFDN_ENABLE_FRAME_POINTER "Add -fno-omit-frame-pointer" ON)
set(MEFDN_MCMODEL "small" CACHE STRING "Memory model specified as -mcmodel=")

set(MEFDN_CXX_STANDARD "0x" CACHE STRING "C++ standard version")

# mefdn-options : INTERFACE target to set options individually

add_library(mefdn-options INTERFACE)

set(MEFDN_GLOBAL_CFLAGS -ftls-model=initial-exec -mcmodel=${MEFDN_MCMODEL})

if (${MEFDN_ENABLE_FRAME_POINTER})
    list(APPEND MEFDN_GLOBAL_CFLAGS -fno-omit-frame-pointer)
endif()

# Publish to all sub-directories
set(MEFDN_GLOBAL_CFLAGS ${MEFDN_GLOBAL_CFLAGS} PARENT_SCOPE)

target_compile_options(mefdn-options INTERFACE
    # C++ Standard
    $<$<COMPILE_LANGUAGE:CXX>:
        $<$<NOT:$<STREQUAL:"${MEFDN_CXX_STANDARD}","">>:-std=c++${MEFDN_CXX_STANDARD}>
    >
    # C Standard
    $<$<COMPILE_LANGUAGE:C>:-std=c99>
    
    #$<$<COMPILE_LANGUAGE:CXX>:-std=c++03> # TODO: COMPILE_LANGUAGE doesn't work on old CMake versions
    -Wno-variadic-macros
    
    ${MEFDN_GLOBAL_CFLAGS}
)
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
    target_compile_options(mefdn-options INTERFACE
        # Optimizations
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:OptDebug>:-O3 -DNDEBUG -g>
        -march=native
        
        -Wall
        -Wextra
        -Wpedantic

        ## Enable -Weverything
        #-Weverything
        #
        ## Disable some warnings of -Weverything
        #-Wno-global-constructors
        #-Wno-exit-time-destructors
        #-Wno-padded # TODO: alignment issues
        #-Wno-disabled-macro-expansion # Problem of stdbool.h
        #-Wno-used-but-marked-unused # e.g. mefdn::try_to_lock
        #-Wno-weak-vtables # couldn't find a correct way to suppress this warning
        #-Wno-c++98-compat
        #-Wno-c++98-compat-pedantic
        #-Wno-abstract-vbase-init # Not approved by C++ standard
        
        # Other options
        #-Wno-c++11-long-long # TODO
        
        # Bug 25546 - Bad diagnostic when using -Wunused-macros and building .S files
        # https://llvm.org/bugs/show_bug.cgi?id=25546
        $<$<COMPILE_LANGUAGE:ASM>:-Wno-unused-macros>
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # Get the version
    set(MEFDN_COMPILER_GCC_VERSION ${CMAKE_CXX_COMPILER_VERSION})
    message(STATUS "MEFDN_COMPILER_GCC_VERSION=${MEFDN_COMPILER_GCC_VERSION}")
    
    CHECK_CXX_COMPILER_FLAG("-Wc++14-compat" MEFDN_GCC_SUPPORTS_WCXX14_COMPAT)
    CHECK_CXX_COMPILER_FLAG("-Wconditionally-supported" MEFDN_GCC_SUPPORTS_WCONDITINALLY_SUPPORTED)
    CHECK_CXX_COMPILER_FLAG("-Wdelete-non-virtual-dtor" MEFDN_GCC_SUPPORTS_WDELETE_NON_VIRTUAL_DTOR)
    CHECK_CXX_COMPILER_FLAG("-Wdouble-promotion" MEFDN_GCC_SUPPORTS_WDOUBLE_PROMOTION)
    CHECK_CXX_COMPILER_FLAG("-Wformat-signedness" MEFDN_GCC_SUPPORTS_WFORMAT_SIGNEDNESS)
    CHECK_CXX_COMPILER_FLAG("-Wnoexcept" MEFDN_GCC_SUPPORTS_WNOEXCEPT)
    CHECK_CXX_COMPILER_FLAG("-Wsuggest-attribute=format" MEFDN_GCC_SUPPORTS_WSUGGEST_ATTRIBUTE)
    CHECK_CXX_COMPILER_FLAG("-Wsuggest-override" MEFDN_GCC_SUPPORTS_WSUGGEST_OVERRIDE)
    #CHECK_CXX_COMPILER_FLAG("-Wuseless-cast" MEFDN_GCC_SUPPORTS_WUSELESS_CAST)
    CHECK_CXX_COMPILER_FLAG("-Wzero-as-null-pointer-constant" MEFDN_GCC_SUPPORTS_WZERO_AS_NULL_POINTER_CONSTANT)
    CHECK_CXX_COMPILER_FLAG("-fmax-errors=1" MEFDN_GCC_SUPPORTS_FMAX_ERRORS)
    CHECK_CXX_COMPILER_FLAG("-no-pie" MEFDN_GCC_SUPPORTS_NO_PIE)
    
    target_compile_options(mefdn-options INTERFACE
        # Optimizations
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:OptDebug>:-O3 -DNDEBUG -g>
        -march=native
        -mavx
        
        # Disable SSP.
        -fno-stack-protector
            # TODO : this is only needed for mgth
        
        # Disable _FORTIFY_SOURCE.
        -D_FORTIFY_SOURCE=0
        
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
        $<$<BOOL:${MEFDN_GCC_SUPPORTS_WCXX14_COMPAT}>:-Wc++14-compat>
        -Wcast-qual
        #-Wchkp # Undocumented warning option
        $<$<BOOL:${MEFDN_GCC_SUPPORTS_WCONDITINALLY_SUPPORTED}>:-Wconditionally-supported>
        -Wconversion
        #$<$<VERSION_GREATER:${MEFDN_COMPILER_GCC_VERSION},4.5>:-Wctor-dtor-privacy> # #pragma GCC diagnostic is required
            # Disabled since it seems useless
        #-Wdate-time # Seems useless
        $<$<BOOL:${MEFDN_GCC_SUPPORTS_WDELETE_NON_VIRTUAL_DTOR}>:-Wdelete-non-virtual-dtor>
        $<$<BOOL:${MEFDN_GCC_SUPPORTS_WDOUBLE_PROMOTION}>:-Wdouble-promotion>
        #-Weffc++ # Out-dated warning
        -Wfloat-equal
        -Wformat-nonliteral
        -Wformat-security
        $<$<BOOL:${MEFDN_GCC_SUPPORTS_WFORMAT_SIGNEDNESS}>:-Wformat-signedness>
        #-Wformat-y2k # Not important
        -Winit-self
        #-Winvalid-pch # TODO
        -Wlogical-op
        -Wmissing-declarations
        -Wmissing-include-dirs
        -Wmultichar
        $<$<BOOL:${MEFDN_GCC_SUPPORTS_WNOEXCEPT}>:-Wnoexcept>
        -Wnon-virtual-dtor
        $<$<VERSION_GREATER:${MEFDN_COMPILER_GCC_VERSION},4.5>:-Wold-style-cast> # #pragma GCC diagnostic is required
        -Woverloaded-virtual
        -Wredundant-decls
        -Wreorder
        -Wsign-promo
        -Wstrict-null-sentinel
        $<$<BOOL:${MEFDN_GCC_SUPPORTS_WSUGGEST_ATTRIBUTE}>:-Wsuggest-attribute=format>
        $<$<BOOL:${MEFDN_GCC_SUPPORTS_WSUGGEST_OVERRIDE}>:-Wsuggest-override>
        -Wswitch-default
        -Wswitch-enum
        -Wsynth
        #-Wsystem-headers # Disable warnings from system headers
        -Wundef
        -Wunused-macros
        #$<$<BOOL:${MEFDN_GCC_SUPPORTS_WUSELESS_CAST}>:-Wuseless-cast>
        -Wwrite-strings
        $<$<BOOL:${MEFDN_GCC_SUPPORTS_WZERO_AS_NULL_POINTER_CONSTANT}>:-Wzero-as-null-pointer-constant>
        
        # Other warnings
        -Wstrict-overflow=5
        
        # Disabled on old versions
        $<$<VERSION_LESS:${MEFDN_COMPILER_GCC_VERSION},4.5>:-Wno-logical-op>
        
        # Prevent too many errors
        #-Wfatal-errors
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    target_compile_options(mefdn-options INTERFACE
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
        -wd2196 # Disable warning #2196: routine is both "inline" and "noinline"
    )
    target_compile_definitions(mefdn-options INTERFACE
        # TODO: workaround to avoid compilation error of fmtlib on ICC
        FMT_DEPRECATED=
    )
endif()

if(MEFDN_PROFILER_USE_GPROF)
    target_compile_options(mefdn-options INTERFACE
        $<$<CXX_COMPILER_ID:GNU>:-pg>
    )
    target_link_libraries(mefdn-options INTERFACE "-pg")
endif()


CHECK_CXX_SOURCE_COMPILES("
__attribute__((always_inline))
void f() { }

int main() { return 0; }
" MEFDN_COMPILER_SUPPORTS_ALWAYS_INLINE)

CHECK_CXX_SOURCE_COMPILES("
void f() { __builtin_unreachable(); }

int main() { return 0; }
" MEFDN_COMPILER_SUPPORTS_BUILTIN_UNREACHABLE)

# To check C++0x features, we need this flag
set(CMAKE_REQUIRED_FLAGS "-std=c++0x")

CHECK_CXX_SOURCE_COMPILES("
struct X { void f() &; void g() &&; };
int main() { return 0; }
" MEFDN_CXX11_REF_QUALIFIERS_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
struct X { explicit operator bool(); };
int main() { return 0; }
" MEFDN_CXX11_EXPLICIT_CONVERSION_OPERATORS_SUPPORTED)

message(STATUS "Generate menps/mefdn/config.h ...")
message(STATUS "BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}")
message(STATUS "MEFDN_PROFILER_USE_GPROF=${MEFDN_PROFILER_USE_GPROF}")
message(STATUS "MEFDN_CXX_STANDARD=${MEFDN_CXX_STANDARD}")
message(STATUS "MEFDN_DISABLE_ALIGN_CHECK_SIZE=${MEFDN_DISABLE_ALIGN_CHECK_SIZE}")

# Generate config.h

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/menps/mefdn/config.h)


# mefdn : SHARED/STATIC target

add_library(mefdn
    ./external/fmt/src/format.cc
    #./external/fmt/src/posix.cc
    ./src/malloc.cpp
    ./src/disable_aslr.cpp
)
target_include_directories(mefdn
    PUBLIC ./include
    PUBLIC ./external/fmt/include
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include # only for mefdn/config.h
)
target_compile_definitions(mefdn
    PUBLIC "$<$<CONFIG:Debug>:MEFDN_DEBUG>"
    PUBLIC "$<$<CONFIG:OptDebug>:MEFDN_DEBUG>"
)
target_link_libraries(mefdn
    PUBLIC ${CMAKE_THREAD_LIBS_INIT} # Add "-lpthread".
    PUBLIC cmpth # now mefdn depends on cmpth
    PRIVATE mefdn-options
    PUBLIC MPI::MPI_CXX
)
install(TARGETS mefdn
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY external/fmt/fmt DESTINATION include)


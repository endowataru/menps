cmake_minimum_required(VERSION 3.0)

project(medsm2)

# Show messages when invoking CMake
message(STATUS "Generate menps/medsm2/config.h ...")

option(MEDSM2_ENABLE_PROF "Enable profiling" FALSE)
if (${MEFDN_ENABLE_MEULT})
    set(MEDSM2_ULT_ITF "MTH" CACHE STRING "ULT module for DSM")
else()
    set(MEDSM2_ULT_ITF "SMTH" CACHE STRING "ULT module for DSM")
endif()
set(MEDSM2_MPI_ITF "QDC" CACHE STRING "MPI module for DSM")
set(MEDSM2_UCT_ITF "QDC" CACHE STRING "UCT module for DSM")
set(MEDSM2_RMA_ITF "MPI" CACHE STRING "RMA module for DSM")
set(MEDSM2_GLOBAL_VAR_BLOCK_SIZE "0x8000" CACHE STRING "Block size of global variables")
set(MEDSM2_MAX_SPACE_SIZE "0x100000000000" CACHE STRING "DSM space size")
set(MEDSM2_MAX_SEG_SIZE "MEDSM2_MAX_SPACE_SIZE / 64" CACHE STRING "Maximum DSM segment size")
set(MEDSM2_LEASE_TS "10" CACHE STRING "Lease value for timestamp-based invalidation")
set(MEDSM2_REL_SIG_LEN "1024" CACHE STRING "Length of write notice array")
set(MEDSM2_WR_COUNT_THRESHOLD "2" CACHE STRING "Threshold for downgrading writable blocks")
set(MEDSM2_MAX_FAST_REL_THRESHOLD "64" CACHE STRING "Threshold for fast release")
option(MEDSM2_ENABLE_FAST_RELEASE "Enable skipping critical sections if data is not migrating" ON)
option(MEDSM2_ENABLE_LAZY_MERGE "Enable lazy merge" ON)
option(MEDSM2_ENABLE_NEEDS_LOCAL_COMP "Reduce local comparison in data merge" OFF)
option(MEDSM2_USE_SIMD_DIFF "Use SIMD to accelerate diff merge" ON)
option(MEDSM2_ENABLE_RACE_DETECTION "Enable dynamic race detection" ON)
option(MEDSM2_ENABLE_MAP_POPULATE "Enable MAP_POPULATE in mmap" OFF)
option(MEDSM2_ENABLE_REL_THREAD "Enable releaser thread" OFF)
# Note: MEDSM2_WR_COUNT_THRESHOLD should be incresed if MEDSM2_ENABLE_REL_THREAD == ON
set(MEDSM2_REL_THREAD_USEC "10000000" CACHE STRING "Fence period for releaser thread")
option(MEDSM2_ENABLE_LINEAR_ALLOCATOR "Use linear allocator" ON)

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/menps/medsm2/config.h)


message(STATUS "Generate linker.ld ...")

# Generate linker.ld
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/linker.ld.in
    ${CMAKE_CURRENT_BINARY_DIR}/linker.ld)


add_library(medsm2
    ./src/sigsegv_catcher.cpp
    ./src/mpi_svm_space.cpp
)
target_include_directories(medsm2
    PUBLIC  ./include
    PUBLIC  ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ./src
)
target_link_libraries(medsm2
    PUBLIC mecom2
    PUBLIC cmpth
    PRIVATE mefdn-options
)



add_library(medsm2-exec-options INTERFACE)

target_link_libraries(medsm2-exec-options
    INTERFACE "-T ${CMAKE_CURRENT_BINARY_DIR}/linker.ld"
    INTERFACE $<$<BOOL:${MEFDN_GCC_SUPPORTS_NO_PIE}>:-no-pie>
)


add_executable(medsm2-ex
    ./example/ex.cpp
)
target_link_libraries(medsm2-ex
    PUBLIC medsm2
    PRIVATE mefdn-options
    PRIVATE medsm2-exec-options
)


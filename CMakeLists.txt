cmake_minimum_required(VERSION 3.0)

project(mgcom)

INCLUDE (CheckIncludeFileCXX)
INCLUDE (CheckCXXSourceCompiles)
INCLUDE (CheckSymbolExists)

add_library(mgcom-header INTERFACE)
target_include_directories(mgcom-header
    INTERFACE ./include
)
target_link_libraries(mgcom-header
    INTERFACE mgbase-header
)

add_library(mgcom-base INTERFACE)
target_include_directories(mgcom-base
    INTERFACE ./src
)
target_link_libraries(mgcom-base INTERFACE mgcom-header mgbase-options)

# Platform Checks

CHECK_INCLUDE_FILE_CXX("mpi.h" is_mpi_available)
#set(is_mpi_available FALSE) # FIXME: Temporary

CHECK_CXX_SOURCE_COMPILES("
#include <mpi.h>

// MPI_VERSION must be greater than 3
int a[MPI_VERSION - 3];

int main() { }
"
is_mpi3_available)
set(is_mpi3_available FALSE) # FIXME: Temporary

CHECK_SYMBOL_EXISTS("FJMPI_Rdma_init" "mpi-ext.h" is_fjmpi_available)
#set(is_fjmpi_available TRUE) # Just for compilation check

# Library for each device

if(${is_mpi_available})
    add_library(mgcom-mpi1
        src/common/rma/region_allocator.cpp
        src/common/rma/malloc.c
        src/device/mpi/command/mpi_command_queue.cpp
        src/device/mpi/rpc/rpc_server.cpp
        src/device/mpi/rpc/rpc_client.cpp
        src/device/mpi/rma/contiguous.cpp
        src/device/mpi/rma/atomic.cpp
        src/device/mpi/rma/rma.cpp
        src/device/mpi/collective/barrier2.cpp
        src/device/mpi/collective/collective.cpp
        src/device/mpi/mpi_base.cpp
        src/device/mpi1/mpi1.cpp
        
        src/device/mpi/rma/poll.cpp
        src/common/rma/try_contiguous.cpp
        src/common/rma/try_atomic.cpp
    )
    target_link_libraries(mgcom-mpi1 mgcom-base mgbase)
    
    list(APPEND mgcom-devices mpi1)
endif()

if(${is_mpi3_available})
    add_library(mgcom-mpi3
        src/common/rma/try_contiguous.cpp
        src/common/rma/try_atomic.cpp
        src/common/rma/region_allocator.cpp
        src/common/rma/malloc.c
        src/device/mpi3/mpi3.cpp
        src/device/mpi3/collective.cpp
        src/device/mpi3/rma/rma.cpp
        src/device/mpi/mpi_base.cpp
        src/device/mpi/mpi_lock.cpp
        src/device/mpi/am/am.cpp
        src/device/mpi/am/sender_queue.cpp
    )
    target_link_libraries(mgcom-mpi3 mgcom-base mgbase)
    
    list(APPEND mgcom-devices mpi3)
endif()

if (${is_fjmpi_available})
    add_library(mgcom-fjmpi
        src/common/rma/region_allocator.cpp
        src/common/rma/malloc.c
        src/common/rma/try_contiguous.cpp
        src/device/mpi/rma/atomic.cpp
        src/device/mpi/mpi_base.cpp
        src/device/mpi/collective/barrier.cpp
        src/device/mpi/collective/collective.cpp
        src/device/mpi/am/am.cpp
        src/device/mpi/am/sender_queue.cpp
        #src/device/fjmpi/rma/rma.cpp
        src/device/fjmpi/rma/queued_rma.cpp
        src/device/fjmpi/fjmpi.cpp
    )
    target_link_libraries(mgcom-fjmpi mgcom-base mgbase)
    
    list(APPEND mgcom-devices fjmpi)
endif()

# Publish the device list

set(mgcom-devices ${mgcom-devices} PARENT_SCOPE)

# Unit test for each device

foreach(device IN LISTS mgcom-devices)
    add_executable(mgcom-unittest-${device}
        ./unittest/unittest.cpp
        #./unittest/am.cpp
        ./unittest/rma.cpp
    )
    target_link_libraries(mgcom-unittest-${device}
        mgcom-${device}
        googletest
    )
endforeach()

# Benchmark programs

foreach(device IN LISTS mgcom-devices)
    file(GLOB benchmark_files example/bench/*.cpp)
    foreach(benchmark_path IN LISTS benchmark_files)
        get_filename_component(benchmark_name ${benchmark_path} NAME_WE)
        add_executable(${benchmark_name}-${device}
            ${benchmark_path}
        )
        target_link_libraries(${benchmark_name}-${device}
            mgcom-${device}
        )
    endforeach()
endforeach()


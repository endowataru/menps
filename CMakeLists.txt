cmake_minimum_required(VERSION 3.0)

project(mgcom)

INCLUDE (CheckIncludeFileCXX)
INCLUDE (CheckCXXSourceCompiles)

add_library(mgcom-header INTERFACE)
target_include_directories(mgcom-header
    INTERFACE ./include
)
target_link_libraries(mgcom-header
    INTERFACE mgbase-header
)

add_library(mgcom-base INTERFACE)
target_link_libraries(mgcom-base INTERFACE mgcom-header mgbase-options)

# Platform Checks

CHECK_INCLUDE_FILE_CXX("mpi.h" is_mpi_available)

CHECK_CXX_SOURCE_COMPILES("
#include <mpi.h>

// MPI_VERSION must be greater than 3
int a[MPI_VERSION - 3];

int main() { }
"
is_mpi3_available)

# Library for each device

if(${is_mpi_available})
    add_library(mgcom-mpi1
        src/common/rma/buffer_pool.cpp
        src/device/mpi1/mpi1.cpp
        src/device/mpi/rma/atomic.cpp
        src/device/mpi/rma/contiguous.cpp
        src/device/mpi/rma/rma.cpp
        src/device/mpi/mpi_base.cpp
        src/device/mpi/collective/barrier.cpp
        src/device/mpi/collective/collective.cpp
        src/device/mpi/am/am.cpp
        src/device/mpi/am/receiver.cpp
        src/device/mpi/am/sender.cpp
        src/device/mpi/am/sender_queue.cpp
    )
    target_include_directories(mgcom-mpi1
        PRIVATE ./src
    )
    target_link_libraries(mgcom-mpi1 mgcom-base mgbase)
    
    list(APPEND mgcom-devices mpi1)
endif()

if(${is_mpi3_available})
    add_library(mgcom-mpi3
        src/common/rma/contiguous.cpp
        src/common/rma/buffer_pool.cpp
        src/device/mpi3/mpi3.cpp
        src/device/mpi3/collective.cpp
        src/device/mpi3/rma/rma.cpp
        src/device/mpi/mpi_base.cpp
        src/device/mpi/am/am.cpp
        src/device/mpi/am/receiver.cpp
        src/device/mpi/am/sender.cpp
        src/device/mpi/am/sender_queue.cpp
    )
    target_include_directories(mgcom-mpi3
        PRIVATE ./src
    )
    target_link_libraries(mgcom-mpi3 mgcom-base mgbase)
    
    list(APPEND mgcom-devices mpi3)
endif()

# Publish the device list

set(mgcom-devices ${mgcom-devices} PARENT_SCOPE)

# Unit test for each device

foreach(device IN LISTS mgcom-devices)
    add_executable(mgcom-unittest-${device}
        ./unittest/unittest.cpp
        ./unittest/am.cpp
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


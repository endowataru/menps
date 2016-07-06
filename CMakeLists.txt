cmake_minimum_required(VERSION 3.0)

project(mgcom)

INCLUDE (CheckIncludeFileCXX)
INCLUDE (CheckCXXSourceCompiles)
INCLUDE (CheckSymbolExists)

add_library(mgcom-header INTERFACE)
target_include_directories(mgcom-header
    INTERFACE ./include
    INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
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
#set(is_mpi_available FALSE) # Just for compilation check

CHECK_CXX_SOURCE_COMPILES("
#include <mpi.h>

// MPI_VERSION must be greater than 3
int a[MPI_VERSION - 3];

int main() { return 0; }
"
MGCOM_DEVICE_MPI3_SUPPORTED)

CHECK_SYMBOL_EXISTS("FJMPI_Rdma_init" "mpi-ext.h" is_fjmpi_available)
#set(is_fjmpi_available TRUE) # Just for compilation check

CHECK_INCLUDE_FILE_CXX("infiniband/verbs.h" is_ibv_available)
#set(is_ibv_available TRUE) # Just for compilation check

CHECK_INCLUDE_FILE_CXX("infiniband/verbs_exp.h" MGCOM_IBV_EXP_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
#include <infiniband/verbs.h>
#include <infiniband/verbs_exp.h>

int x = IBV_EXP_DEVICE_ATTR_EXT_ATOMIC_ARGS;

int main() { return 0; } "
MGCOM_IBV_MASKED_ATOMICS_SUPPORTED)


# Library for each device

if(${is_mpi_available})
    add_library(mgcom-mpi1
        src/common/rma/region_allocator.cpp
        src/common/rma/malloc.cpp
        src/device/mpi/rpc/rpc.cpp
        src/device/mpi/rma/contiguous.cpp
        src/device/mpi/rma/atomic.cpp
        src/device/mpi/rma/rma.cpp
        src/device/mpi/collective/barrier.cpp
        src/device/mpi/mpi_base.cpp
        src/device/mpi/endpoint.cpp
        src/device/mpi/command/mpi_delegator.cpp
        src/device/mpi1/command/commander.cpp
        src/device/mpi1/mpi1.cpp
        src/common/entrypoint.cpp
        src/device/mpi/mpi_communicator.cpp
    )
    target_link_libraries(mgcom-mpi1 mgcom-base mgbase)
    
    list(APPEND mgcom-devices mpi1)
endif()

if(${MGCOM_DEVICE_MPI3_SUPPORTED})
    add_library(mgcom-mpi3
        src/common/rma/region_allocator.cpp
        src/common/rma/malloc.cpp
        src/device/mpi/rpc/rpc.cpp
        src/device/mpi/mpi_base.cpp
        src/device/mpi/endpoint.cpp
        src/device/mpi/command/mpi_delegator.cpp
        src/device/mpi3/command/mpi3_delegator.cpp
        src/device/mpi3/rma/rma_window.cpp
        src/device/mpi3/command/commander.cpp
        src/device/mpi3/mpi3.cpp
        src/device/mpi3/entrypoint.cpp
        src/device/mpi3/collective/collective.cpp
        src/device/mpi/mpi_communicator.cpp
        src/device/mpi3/rma/requester.cpp
        src/device/mpi3/rma/registrator.cpp
    )
    target_link_libraries(mgcom-mpi3 mgcom-base mgbase)
    
    list(APPEND mgcom-devices mpi3)
endif()

if (${is_fjmpi_available})
    add_library(mgcom-fjmpi
        src/common/rma/region_allocator.cpp
        src/common/rma/malloc.cpp
        src/device/mpi/rma/atomic.cpp
        src/device/mpi/mpi_base.cpp
        src/device/mpi/endpoint.cpp
        src/device/mpi/collective/barrier.cpp
        src/device/mpi/command/mpi_delegator.cpp
        #src/device/fjmpi/command/fjmpi_command_queue.cpp
        src/device/fjmpi/fjmpi.cpp
        src/device/fjmpi/rma/requester.cpp
        src/device/fjmpi/rma/registrator.cpp
        src/device/fjmpi/rpc/rpc.cpp
        src/device/fjmpi/rpc/rpc_sender.cpp
        src/device/fjmpi/rpc/rpc_receiver.cpp
        src/device/fjmpi/rpc/rpc_connection_pool.cpp
        src/device/fjmpi/entrypoint.cpp
        src/device/fjmpi/endpoint.cpp
        
        src/device/fjmpi/scheduler/scheduler.cpp
        src/device/fjmpi/scheduler/fjmpi_delegator.cpp
        src/device/fjmpi/scheduler/command_consumer.cpp
    )
    target_link_libraries(mgcom-fjmpi mgcom-base mgbase)
    
    list(APPEND mgcom-devices fjmpi)
endif()

if (${is_ibv_available})
    add_library(mgcom-ibv
        src/device/ibv/entrypoint.cpp
        src/device/ibv/ibv.cpp
        
        src/common/rma/region_allocator.cpp
        src/common/rma/malloc.cpp
        
        src/device/mpi/rpc/rpc.cpp
        src/device/mpi/collective/barrier.cpp
        src/device/mpi/mpi_base.cpp
        src/device/mpi/endpoint.cpp
        src/device/mpi1/command/commander.cpp
        src/device/mpi/command/mpi_delegator.cpp
        
        src/device/ibv/rma/direct_requester.cpp
        src/device/ibv/rma/rma_comm.cpp
        src/device/ibv/rma/registrator.cpp
        src/device/ibv/command/poll_thread.cpp
        src/device/ibv/native/queue_pair.cpp
        src/device/ibv/native/alltoall_queue_pairs.cpp
        
        src/device/ibv/scheduler/command_consumer.cpp
    )
    target_link_libraries(mgcom-ibv mgcom-base mgbase
        ibverbs # libibverbs.so
    )
    
    list(APPEND mgcom-devices ibv)
endif()


#add_library(mgcom-uni
#    src/common/rma/region_allocator.cpp
#    src/common/rma/malloc.cpp
#    
#    src/device/mpi/rpc/rpc.cpp
#    src/device/mpi/rma/contiguous.cpp
#    src/device/mpi/rma/atomic.cpp
#    src/device/mpi/rma/rma.cpp
#    src/device/mpi/collective/barrier.cpp
#    src/device/mpi/mpi_base.cpp
#    src/device/mpi/command/mpi_delegator.cpp
#    src/device/mpi1/command/commander.cpp
#    src/device/mpi1/mpi1.cpp
#    src/common/entrypoint.cpp
#    $<$<BOOL:${MGCOM_DEVICE_MPI3_SUPPORTED}>:
#        src/device/mpi3/command/mpi3_command_queue.cpp
#        src/device/mpi3/mpi3.cpp
#        src/device/mpi3/rma/rma.cpp>
#)
#target_link_libraries(mgcom-uni mgcom-base mgbase)
#list(APPEND mgcom-devices uni)

# Generate config.h

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/mgcom/config.h)

# Publish the device list

set(mgcom-devices ${mgcom-devices} PARENT_SCOPE)

# Unit test for each device

foreach(device IN LISTS mgcom-devices)
    add_executable(mgcom-unittest-${device}
        ./unittest/unittest.cpp
        ./unittest/rpc.cpp
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

# Raw Benchmark

#if (${is_fjmpi_available})
#    add_executable(raw-latency
#        example/raw/latency.cpp
#    )
#    target_link_libraries(raw-latency mgbase)
#endif()


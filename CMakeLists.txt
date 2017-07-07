cmake_minimum_required(VERSION 3.0)

project(mgdev)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)
include(CheckSymbolExists)

# Options

option(MGDEV_DEVICE_UCX_ENABLED "Enable UCX" TRUE)
set(MGDEV_ULT_NAMESPACE "::mgult::klt" CACHE STRING "Namespace for ult" )

# Platform check

CHECK_INCLUDE_FILE_CXX("mpi.h" MGDEV_DEVICE_MPI_SUPPORTED)
#set(MGDEV_DEVICE_MPI_SUPPORTED FALSE) # Comment out for compilation check


CHECK_CXX_SOURCE_COMPILES("
#include <mpi.h>

// MPI_VERSION must be greater than 3
int a[MPI_VERSION - 3];

int main() { return 0; }
"
MGDEV_DEVICE_MPI3_SUPPORTED)

CHECK_INCLUDE_FILE_CXX("infiniband/verbs.h" MGDEV_DEVICE_IBV_SUPPORTED)
#set(MGDEV_DEVICE_IBV_SUPPORTED TRUE) # Comment out for compilation check

CHECK_INCLUDE_FILE_CXX("infiniband/verbs_exp.h" MGDEV_IBV_EXP_SUPPORTED)

CHECK_CXX_SOURCE_COMPILES("
#include <infiniband/verbs.h>
#include <infiniband/verbs_exp.h>

int x = IBV_EXP_DEVICE_ATTR_EXT_ATOMIC_ARGS;

int main() { return 0; } "
MGDEV_IBV_MASKED_ATOMICS_SUPPORTED)

CHECK_SYMBOL_EXISTS("FJMPI_Rdma_init" "mpi-ext.h" MGDEV_DEVICE_FJMPI_SUPPORTED)
#set(MGDEV_DEVICE_FJMPI_SUPPORTED TRUE) # Just for compilation check

# Export variables to the parent scope (for mgcom).
set(MGDEV_DEVICE_MPI_SUPPORTED ${MGDEV_DEVICE_MPI_SUPPORTED} PARENT_SCOPE)
set(MGDEV_DEVICE_MPI3_SUPPORTED ${MGDEV_DEVICE_MPI3_SUPPORTED} PARENT_SCOPE)
set(MGDEV_DEVICE_IBV_SUPPORTED ${MGDEV_DEVICE_IBV_SUPPORTED} PARENT_SCOPE)
set(MGDEV_IBV_EXP_SUPPORTED ${MGDEV_IBV_EXP_SUPPORTED} PARENT_SCOPE)
set(MGDEV_IBV_MASKED_ATOMICS_SUPPORTED ${MGDEV_IBV_MASKED_ATOMICS_SUPPORTED} PARENT_SCOPE)
set(MGDEV_DEVICE_FJMPI_SUPPORTED ${MGDEV_DEVICE_FJMPI_SUPPORTED} PARENT_SCOPE)

# Generate config.h

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/mgdev/config.h)


if(${MGDEV_DEVICE_MPI_SUPPORTED})
    add_library(mgdev-mpi
        ./src/mpi/environment.cpp
        ./src/mpi/mpi_error.cpp
        ./src/mpi/communicator.cpp
        ./src/mpi/mpi1_requester.cpp
        $<$<BOOL:${MGDEV_DEVICE_MPI3_SUPPORTED}>:./src/mpi/mpi3_requester.cpp>
    )
    target_include_directories(mgdev-mpi
        PUBLIC  ./include
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
        PRIVATE ./src
    )
    target_link_libraries(mgdev-mpi
        PUBLIC mgult
        PRIVATE mgbase-options
    )
    install(TARGETS mgdev-mpi
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

if(${MGDEV_DEVICE_IBV_SUPPORTED})
    add_library(mgdev-ibv
        ./src/ibv/ibv_error.cpp
        ./src/ibv/attributes.cpp
        ./src/ibv/queue_pair.cpp
        ./src/ibv/completion_queue.cpp
        ./src/ibv/device_list.cpp
        ./src/ibv/device_context.cpp
        ./src/ibv/memory_region.cpp
        ./src/ibv/protection_domain.cpp
    )
    target_include_directories(mgdev-ibv
        PUBLIC  ./include
        PUBLIC  ${CMAKE_CURRENT_BINARY_DIR}
        PRIVATE ./src
    )
    target_link_libraries(mgdev-ibv
        PUBLIC mgult
        PRIVATE mgbase-options
        PRIVATE ibverbs # libibverbs.so
    )
    install(TARGETS mgdev-ibv
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

if(${MGDEV_DEVICE_UCX_ENABLED})
    include(cmake/ucx.cmake)
    
    # UCT
    
    add_library(mgdev-uct
        ./src/ucx/ucs/async_context.cpp
        ./src/ucx/uct/iface_config.cpp
        ./src/ucx/uct/interface.cpp
        ./src/ucx/uct/md_config.cpp
        ./src/ucx/uct/md_resource_list.cpp
        ./src/ucx/uct/tl_resource_list.cpp
        ./src/ucx/uct/memory_domain.cpp
        ./src/ucx/uct/worker.cpp
        ./src/ucx/uct/allocated_memory.cpp
        ./src/ucx/uct/endpoint.cpp
        ./src/ucx/ucx_error.cpp
    )
    target_include_directories(mgdev-uct
        PUBLIC  ./include
        PUBLIC  ${CMAKE_CURRENT_BINARY_DIR}
        PRIVATE ./src
    )
    target_link_libraries(mgdev-uct
        PUBLIC  uct
        PUBLIC  ucs
        PUBLIC  mgult
        PRIVATE mgbase-options
    )
    
    add_executable(mgdev-ucx_uct_test
        ./examples/ucx/uct/uct_test.cpp
    )
    target_link_libraries(mgdev-ucx_uct_test
        mgdev-uct
        mgbase-options
    )
    
    # UCP
    
    add_library(mgdev-ucp
        ./src/ucx/ucp/context.cpp
        ./src/ucx/ucp/config.cpp
        ./src/ucx/ucp/worker.cpp
        ./src/ucx/ucp/worker_address.cpp
        ./src/ucx/ucp/endpoint.cpp
        ./src/ucx/ucp/rkey_buffer.cpp
        ./src/ucx/ucp/remote_key.cpp
        ./src/ucx/ucp/memory.cpp
        ./src/ucx/ucx_error.cpp
    )
    target_include_directories(mgdev-ucp
        PUBLIC  ./include
        PUBLIC  ${CMAKE_CURRENT_BINARY_DIR}
        PRIVATE ./src
    )
    target_link_libraries(mgdev-ucp
        PUBLIC  ucp
        PUBLIC  ucs
        PUBLIC  mgult
        PRIVATE mgbase-options
    )
    
    add_executable(mgdev-ucx_ucp_test
        ./examples/ucx/ucp/ucp_test.cpp
    )
    target_link_libraries(mgdev-ucx_ucp_test
        mgdev-ucp
        mgbase-options
    )
endif()

# Examples

if(${MGDEV_DEVICE_IBV_SUPPORTED})
    add_executable(mgdev-ibv_test
        ./examples/ibv/ibv_test.cpp
    )
    target_link_libraries(mgdev-ibv_test
        mgdev-ibv
        mgbase-options
    )
    
    add_executable(mgdev-ibv_qp_count
        ./examples/ibv/ibv_qp_count.cpp
    )
    target_link_libraries(mgdev-ibv_qp_count
        mgdev-ibv
        mgbase-options
    )
endif()


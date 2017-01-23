cmake_minimum_required(VERSION 3.0)

project(mgdev)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)
include(CheckSymbolExists)

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

option(MGDEV_DEVICE_UCX_ENABLED "Enable UCX" FALSE)
#set(MGDEV_DEVICE_UCX_ENABLED TRUE) # Comment out to use


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
        PRIVATE ./src
    )
    target_link_libraries(mgdev-mpi
        mgbase-options
        mgult
    )
endif()

if(${MGDEV_DEVICE_IBV_SUPPORTED})
    add_library(mgdev-ibv
        ./src/ibv/queue_pair.cpp
    )
    target_include_directories(mgdev-ibv
        PUBLIC  ./include
        PRIVATE ./src
    )
    target_link_libraries(mgdev-ibv
        mgbase-options
        mgult
    )
endif()

if(${MGDEV_DEVICE_UCX_ENABLED})
    include(cmake/ucx.cmake)
endif()



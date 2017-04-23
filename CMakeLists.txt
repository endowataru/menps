cmake_minimum_required(VERSION 3.0)

project(mgcom)

include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)
include(CheckSymbolExists)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

# Options
option(MGCOM_ENABLE_LOOPBACK_RMA "Enable loopback RMA" OFF)
option(MGCOM_IBV_ENABLE_SLEEP "Enable sleep on IBV" OFF)
option(MGCOM_IBV_SEPARATE_CQ "Separate CQ into QPs" OFF)

# mgcom-mpi : MPI-based implementation

add_library(mgcom-mpi
    src/common/rma/default_allocator.cpp
    src/device/mpi/rpc/rpc.cpp
    src/device/mpi/rma/rma.cpp
    src/device/mpi/rma/registrator.cpp
    src/device/mpi/collective/barrier.cpp
    src/device/mpi/mpi_base.cpp
    src/device/mpi/endpoint.cpp
    src/device/mpi/command/mpi_delegator.cpp
    src/device/mpi/mpi_communicator.cpp
)
target_include_directories(mgcom-mpi
    PUBLIC ./include
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ./src
)
target_link_libraries(mgcom-mpi
    PUBLIC mgdev-mpi
    PRIVATE mgbase-options
)
install(TARGETS mgcom-mpi
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Library for each device

if(${MGDEV_DEVICE_MPI_SUPPORTED})
    add_library(mgcom-dev-mpi1
        src/device/mpi1/command/commander.cpp
        src/device/mpi1/mpi1.cpp
    )
    target_include_directories(mgcom-dev-mpi1
        PRIVATE ./src
    )
    target_link_libraries(mgcom-dev-mpi1
        PUBLIC mgcom-mpi
        PRIVATE mgbase-options
    )
    install(TARGETS mgcom-dev-mpi1
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )
endif()

if(${MGDEV_DEVICE_MPI3_SUPPORTED})
    add_library(mgcom-dev-mpi3
        src/device/mpi3/command/mpi3_delegator.cpp
        src/device/mpi3/rma/rma_window.cpp
        src/device/mpi3/command/commander.cpp
        src/device/mpi3/collective/collective.cpp
        src/device/mpi3/rma/requester.cpp
        src/device/mpi3/rma/registrator.cpp
        src/device/mpi3/mpi3.cpp
    )
    target_include_directories(mgcom-dev-mpi3
        PRIVATE ./src
    )
    target_link_libraries(mgcom-dev-mpi3
        PUBLIC mgcom-mpi
        PRIVATE mgbase-options
    )
    install(TARGETS mgcom-dev-mpi3
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

if (${MGDEV_DEVICE_FJMPI_SUPPORTED})
    add_library(mgcom-dev-fjmpi
        src/device/fjmpi/fjmpi.cpp
        src/device/fjmpi/rma/requester.cpp
        src/device/fjmpi/rma/registrator.cpp
        src/device/fjmpi/rpc/rpc.cpp
        src/device/fjmpi/rpc/rpc_sender.cpp
        src/device/fjmpi/rpc/rpc_receiver.cpp
        src/device/fjmpi/rpc/rpc_connection_pool.cpp
        src/device/fjmpi/endpoint.cpp
        src/device/fjmpi/scheduler/scheduler.cpp
        src/device/fjmpi/scheduler/fjmpi_delegator.cpp
        src/device/fjmpi/scheduler/command_consumer.cpp
    )
    target_include_directories(mgcom-dev-fjmpi
        PRIVATE ./src
    )
    target_link_libraries(mgcom-dev-fjmpi
        PUBLIC mgcom-mpi
        PRIVATE mgbase-options
    )
    install(TARGETS mgcom-dev-fjmpi
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )
endif()

if (${MGDEV_DEVICE_IBV_SUPPORTED})
    add_library(mgcom-dev-ibv
        src/device/ibv/rma/direct_requester.cpp
        src/device/ibv/rma/rma_comm.cpp
        src/device/ibv/rma/registrator.cpp
        src/device/ibv/command/poll_thread.cpp
        src/device/ibv/native/alltoall_queue_pairs.cpp
        src/device/ibv/scheduler/command_consumer.cpp
        src/device/ibv/scheduler/scheduler.cpp
        src/device/ibv/ibv_starter.cpp
    )
    target_include_directories(mgcom-dev-ibv
        PRIVATE ./src
    )
    target_link_libraries(mgcom-dev-ibv 
        PUBLIC mgcom-dev-mpi1
        PRIVATE mgdev-ibv
        PRIVATE mgbase-options
    )
    
    list(APPEND mgcom-devices ibv)
    
    install(TARGETS mgcom-dev-ibv
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )
endif()


add_library(mgcom
    src/common/entrypoint.cpp
)
target_include_directories(mgcom
    PRIVATE ./src
)
target_link_libraries(mgcom
    PUBLIC $<$<BOOL:${MGDEV_DEVICE_MPI_SUPPORTED}>:mgcom-dev-mpi1>
    PUBLIC $<$<BOOL:${MGDEV_DEVICE_MPI3_SUPPORTED}>:mgcom-dev-mpi3>
    PUBLIC $<$<BOOL:${MGDEV_DEVICE_IBV_SUPPORTED}>:mgcom-dev-ibv>
    PUBLIC $<$<BOOL:${MGDEV_DEVICE_FJMPI_SUPPORTED}>:mgcom-dev-fjmpi>
    PRIVATE mgbase-options
)
install(TARGETS mgcom
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Generate config.h

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/mgcom/config.h)

# Unit test for each device

add_executable(mgcom-unittest
    ./unittest/unittest.cpp
    ./unittest/rpc.cpp
    ./unittest/rma.cpp
)
target_link_libraries(mgcom-unittest
    mgcom
    mgbase-options
    googletest
)

# Benchmark programs

file(GLOB benchmark_files example/bench/*.cpp)
foreach(benchmark_path IN LISTS benchmark_files)
    get_filename_component(benchmark_name ${benchmark_path} NAME_WE)
    add_executable(${benchmark_name}
        ${benchmark_path}
    )
    target_link_libraries(${benchmark_name}
        mgcom
        mgbase-options
    )
endforeach()


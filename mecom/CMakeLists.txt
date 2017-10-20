cmake_minimum_required(VERSION 3.0)

project(mecom)

include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)
include(CheckSymbolExists)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

# Options
option(MECOM_ENABLE_LOOPBACK_RMA "Enable loopback RMA" OFF)
option(MECOM_IBV_ENABLE_SLEEP_QP "Enable QP sleep on IBV" OFF)
option(MECOM_IBV_ENABLE_SLEEP_CQ "Enable CQ sleep on IBV" OFF)
option(MECOM_IBV_SEPARATE_CQ "Separate CQ into QPs" OFF)

option(MECOM_USE_SLEEP_RMA "Enable sleeping thread in RMA" ON)
option(MECOM_FORK_EXECUTOR_THREAD "Use ult::thread to start offloading" OFF)
option(MECOM_FORK_COMPLETER_THREAD "Use ult::thread to start offloading" OFF)

set(MECOM_IBV_SEND_INLINE_SIZE "92" CACHE STRING "Use IBV_SEND_INLINE below this size")
#set(MECOM_IBV_SEND_INLINE_SIZE "0 " CACHE STRING "Use IBV_SEND_INLINE below this size")

set(MECOM_EXECUTOR_LIFETIME_CYCLES "0 " CACHE STRING "Run executors during at least this time")

# mecom-mpi : MPI-based implementation

add_library(mecom-mpi
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
target_include_directories(mecom-mpi
    PUBLIC ./include
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ./src
)
target_link_libraries(mecom-mpi
    PUBLIC medev-mpi
    PRIVATE mefdn-options
)
install(TARGETS mecom-mpi
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Library for each device

if(${MEDEV_DEVICE_MPI_SUPPORTED})
    add_library(mecom-dev-mpi1
        src/device/mpi1/command/commander.cpp
        src/device/mpi1/mpi1.cpp
    )
    target_include_directories(mecom-dev-mpi1
        PRIVATE ./src
    )
    target_link_libraries(mecom-dev-mpi1
        PUBLIC mecom-mpi
        PRIVATE mefdn-options
    )
    install(TARGETS mecom-dev-mpi1
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

if(${MEDEV_DEVICE_MPI3_SUPPORTED})
    add_library(mecom-dev-mpi3
        src/device/mpi3/command/mpi3_delegator.cpp
        src/device/mpi3/rma/rma_window.cpp
        src/device/mpi3/command/commander.cpp
        src/device/mpi3/collective/collective.cpp
        src/device/mpi3/rma/requester.cpp
        src/device/mpi3/rma/registrator.cpp
        src/device/mpi3/mpi3.cpp
    )
    target_include_directories(mecom-dev-mpi3
        PRIVATE ./src
    )
    target_link_libraries(mecom-dev-mpi3
        PUBLIC mecom-mpi
        PRIVATE mefdn-options
    )
    install(TARGETS mecom-dev-mpi3
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

if (${MEDEV_DEVICE_FJMPI_SUPPORTED})
    add_library(mecom-dev-fjmpi
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
    target_include_directories(mecom-dev-fjmpi
        PRIVATE ./src
    )
    target_link_libraries(mecom-dev-fjmpi
        PUBLIC mecom-mpi
        PRIVATE mefdn-options
    )
    install(TARGETS mecom-dev-fjmpi
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

if (${MEDEV_DEVICE_IBV_SUPPORTED})
    add_library(mecom-dev-ibv
        src/device/ibv/rma/direct_requester.cpp
        src/device/ibv/rma/rma_comm.cpp
        src/device/ibv/rma/registrator.cpp
        src/device/ibv/command/poll_thread.cpp
        src/device/ibv/native/alltoall_queue_pairs.cpp
        src/device/ibv/scheduler/command_consumer.cpp
        src/device/ibv/scheduler/scheduler.cpp
        src/device/ibv/ibv_starter.cpp
        src/device/ibv/offload/rma_executor.cpp
        src/device/ibv/offload/rma_offload_requester.cpp
    )
    target_include_directories(mecom-dev-ibv
        PRIVATE ./src
    )
    target_link_libraries(mecom-dev-ibv 
        PUBLIC mecom-dev-mpi1
        PRIVATE medev-ibv
        PRIVATE mefdn-options
    )
    
    list(APPEND mecom-devices ibv)
    
    install(TARGETS mecom-dev-ibv
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()


add_library(mecom
    src/common/entrypoint.cpp
)
target_include_directories(mecom
    PRIVATE ./src
)
target_link_libraries(mecom
    PUBLIC $<$<BOOL:${MEDEV_DEVICE_MPI_SUPPORTED}>:mecom-dev-mpi1>
    PUBLIC $<$<BOOL:${MEDEV_DEVICE_MPI3_SUPPORTED}>:mecom-dev-mpi3>
    PUBLIC $<$<BOOL:${MEDEV_DEVICE_IBV_SUPPORTED}>:mecom-dev-ibv>
    PUBLIC $<$<BOOL:${MEDEV_DEVICE_FJMPI_SUPPORTED}>:mecom-dev-fjmpi>
    PRIVATE mefdn-options
)
install(TARGETS mecom
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Generate config.h

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/menps/mecom/config.h)

# Unit test for each device

add_executable(mecom-unittest
    ./unittest/unittest.cpp
    ./unittest/rpc.cpp
    ./unittest/rma.cpp
)
target_link_libraries(mecom-unittest
    mecom
    mefdn-options
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
        mecom
        mefdn-options
    )
endforeach()


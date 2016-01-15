cmake_minimum_required(VERSION 3.0)

project(mgcom)

add_library(mgcom-header INTERFACE)
target_include_directories(mgcom-header
    INTERFACE ./include
)
target_link_libraries(mgcom-header
    INTERFACE mgbase-header
)

add_library(mgcom-mpi3
    src/common/polling.cpp
    src/common/mpi_base.cpp
    src/common/rma/contiguous.cpp
    src/common/rma/buffer_pool.cpp
    src/device/mpi3/mpi3.cpp
    src/device/mpi3/collective.cpp
    src/device/mpi3/rma/rma.cpp
    src/device/mpi3/am/am.cpp
    src/device/mpi3/am/receiver.cpp
    src/device/mpi3/am/sender.cpp
    src/device/mpi3/am/sender_queue.cpp
)

target_include_directories(mgcom-mpi3
    PRIVATE ./src
)

target_link_libraries(mgcom-mpi3 mgcom-header mgbase)

set(mgcom-devices mpi3 PARENT_SCOPE)


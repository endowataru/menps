cmake_minimum_required(VERSION 3.0)

project(mgcom)

add_library(mgcom-header INTERFACE)
target_include_directories(mgcom-header
    INTERFACE ./include
)
target_link_libraries(mgcom-header
    INTERFACE mgbase-header
)

add_library(mgcom-base INTERFACE)
target_link_libraries(mgcom-base INTERFACE mgcom-header)
target_compile_options(mgcom-base INTERFACE
    "-Wall"
    "-std=c++03"
    "-pedantic"
    "-Wextra"
    "-Wcast-qual"
    "-Wctor-dtor-privacy"
    "-Wdisabled-optimization"
    "-Wformat=2"
    "-Winit-self"
    "-Wmissing-declarations"
    "-Wmissing-include-dirs"
    "-Woverloaded-virtual"
    "-Wredundant-decls"
    "-Wshadow"
    "-Wsign-promo"
    "-Wstrict-overflow=5"
    "-Wswitch-default"
    "-Wno-c++11-long-long"
    "-Wno-variadic-macros"
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

target_link_libraries(mgcom-mpi3 mgcom-base mgbase)

set(mgcom-devices mpi3)
set(mgcom-devices ${mgcom-devices} PARENT_SCOPE)

foreach (device IN LISTS mgcom-devices)
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


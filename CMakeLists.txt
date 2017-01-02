cmake_minimum_required(VERSION 3.0)

project(mgdev)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

add_library(mgdev-mpi
    ./src/mpi/environment.cpp
    ./src/mpi/mpi_error.cpp
    ./src/mpi/communicator.cpp
    ./src/mpi/mpi1_requester.cpp
    ./src/mpi/mpi3_requester.cpp
)
target_include_directories(mgdev-mpi
    PUBLIC  ./include
    PRIVATE ./src
)
target_link_libraries(mgdev-mpi
    mgbase-options
    mgult
)


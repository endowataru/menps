cmake_minimum_required(VERSION 3.0)

project(meqdc)

# Add RPATH in installation
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(MEQDC_MPI_ULT_ITF "SCT" CACHE STRING "Default ULT interface for meqdc-wrap-mpi")
option(MEQDC_MPI_ENABLE_STATIC_OFFLOADING "Enable static offloading in MPI" FALSE)
set(MEQDC_MPI_REQUEST_POOL_MAX_ENTRIES 128 CACHE STRING "Pool threshold for MPI requests")
set(MEQDC_MPI_DELEGATOR_POOL_MAX_ENTRIES 128 CACHE STRING "Pool threshold for MPI delegator")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/menps/meqdc/config.h)

add_library(meqdc INTERFACE)

target_include_directories(meqdc INTERFACE
    ./include
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(meqdc INTERFACE
    medev2
    mefdn-options
)
install(TARGETS meqdc
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)

add_library(meqdc-wrap-mpi
    src/wrap_mpi.cpp
)
target_link_libraries(meqdc-wrap-mpi
    PUBLIC meqdc
    PRIVATE mefdn-options
)


file(GLOB example_files example/mpi/*.cpp)
foreach(example_path IN LISTS example_files)
    get_filename_component(example_name ${example_path} NAME_WE)
    add_executable(meqdc-mpi-${example_name}
        ${example_path}
    )
    target_link_libraries(meqdc-mpi-${example_name}
        PUBLIC meqdc-wrap-mpi
        PRIVATE mefdn-options
    )
endforeach()


cmake_minimum_required(VERSION 3.0)

project(meqdc)

set(MEQDC_MPI_ULT_ITF "MTH" CACHE STRING "Default ULT interface for meqdc-wrap-mpi")
option(MEQDC_MPI_ENABLE_STATIC_OFFLOADING "Enable static offloading in MPI" FALSE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/menps/meqdc/config.h)

add_library(meqdc
    src/compile.cpp
)

target_include_directories(meqdc
    PUBLIC ./include
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(meqdc
    PUBLIC medev2
    PRIVATE mefdn-options
    PUBLIC meuct
        # TODO: remove dependency
)

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


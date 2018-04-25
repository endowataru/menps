cmake_minimum_required(VERSION 3.0)

project(medev2)

# Show messages when invoking CMake
message(STATUS "Generate menps/medev2/config.h ...")

set(MEDEV2_DEFAULT_ULT_HEADER "<menps/meult/klt.hpp>" CACHE STRING
    "Ult header for menps::medev2::default_ult_itf")

set(MEDEV2_DEFAULT_ULT_ITF "::menps::meult::klt_policy" CACHE STRING
    "Ult interface as menps::medev2::default_ult_itf" )

option(MEDEV2_SERIALIZE_MPI_CALLS "Serialize MPI calls" TRUE)

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/menps/medev2/config.h)


add_library(medev2 INTERFACE)

target_include_directories(medev2
    INTERFACE ./include
    INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(medev2
    INTERFACE meult
)


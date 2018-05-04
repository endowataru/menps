cmake_minimum_required(VERSION 3.0)

project(medsm2)

# Show messages when invoking CMake
message(STATUS "Generate menps/medsm2/config.h ...")

option(MEDSM2_ENABLE_PROF "Enable profiling" FALSE)

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/menps/medsm2/config.h)


add_library(medsm2
    ./src/sigsegv_catcher.cpp
    ./src/mpi_svm_space.cpp
)
target_include_directories(medsm2
    PUBLIC  ./include
    PUBLIC  ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ./src
)
target_link_libraries(medsm2
    PUBLIC mecom2
    PRIVATE mefdn-options
)



add_library(medsm2-exec-options INTERFACE)

target_link_libraries(medsm2-exec-options
    INTERFACE "-T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld"
    INTERFACE $<$<BOOL:${MEFDN_GCC_SUPPORTS_NO_PIE}>:-no-pie>
)


add_executable(medsm2-ex
    ./example/ex.cpp
)
target_link_libraries(medsm2-ex
    PUBLIC medsm2
    PRIVATE mefdn-options
    PRIVATE medsm2-exec-options
)


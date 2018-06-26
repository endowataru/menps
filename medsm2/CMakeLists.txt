cmake_minimum_required(VERSION 3.0)

project(medsm2)

# Show messages when invoking CMake
message(STATUS "Generate menps/medsm2/config.h ...")

option(MEDSM2_ENABLE_PROF "Enable profiling" FALSE)
option(MEDSM2_USE_UCT_RMA "Use UCT RMA" FALSE)
option(MEDSM2_USE_UCP_RMA "Use UCP RMA" FALSE)
set(MEDSM2_GLOBAL_VAR_BLOCK_SIZE "0x1000" CACHE STRING "Block size of global variables")

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/menps/medsm2/config.h)


message(STATUS "Generate linker.ld ...")

# Generate linker.ld
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/linker.ld.in
    ${CMAKE_CURRENT_BINARY_DIR}/linker.ld)


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
    INTERFACE "-T ${CMAKE_CURRENT_BINARY_DIR}/linker.ld"
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


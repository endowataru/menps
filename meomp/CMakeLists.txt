cmake_minimum_required(VERSION 3.0)

project(meomp)

set(MEOMP_DEF_STKSIZE 32768 CACHE STRING "Default stack size")
set(MEOMP_HEAP_SIZE 1048576 CACHE STRING "Default heap size")
set(MEOMP_HEAP_BLOCK_SIZE 32768 CACHE STRING "Default heap block size")
option(MEOMP_ENABLE_QDLOCK_BARRIER "Use qdlock_barrier in meomp" ON)

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/menps/meomp/config.h)

add_library(meomp STATIC
    ./src/meomp.cpp
)
target_include_directories(meomp
    PUBLIC ./include
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)
target_compile_options(meomp
    PUBLIC -DMEOMP_ENABLED
)
target_link_libraries(meomp
    PUBLIC medsm2
    PRIVATE mefdn-options
    PRIVATE mectx
)
install(TARGETS meomp
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)

file(GLOB example_files example/*.cpp)
foreach(example_path IN LISTS example_files)
    get_filename_component(example_name ${example_path} NAME_WE)
    add_executable(meomp-${example_name}
        ${example_path}
    )
    target_compile_options(meomp-${example_name}
        PRIVATE -fopenmp
    )
    target_link_libraries(meomp-${example_name}
        PUBLIC meomp
        PRIVATE mefdn-options
        PRIVATE medsm2-exec-options
    )
endforeach()


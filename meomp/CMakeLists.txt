cmake_minimum_required(VERSION 3.0)

project(meomp)

set(MEOMP_DEF_STKSIZE 32768 CACHE STRING "Default stack size")
set(MEOMP_HEAP_SIZE 1048576 CACHE STRING "Default heap size")
set(MEOMP_HEAP_BLOCK_SIZE 32768 CACHE STRING "Default heap block size")
option(MEOMP_ENABLE_MEDSM3 "Enable medsm3 for meomp" OFF)
option(MEOMP_ENABLE_BLAS "Enable BLAS benchmark" OFF)

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
    PUBLIC medsm3
    PRIVATE mefdn-options
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

if (${MEOMP_ENABLE_BLAS})
    set(BLA_VENDOR Intel10_64ilp_seq)
    find_package(BLAS)

    add_executable(meomp-mm-blas
        example/mm.cpp
    )
    target_compile_options(meomp-mm-blas
        PRIVATE -fopenmp
    )
    target_compile_definitions(meomp-mm-blas
        PUBLIC BENCH_USE_BLAS
        PUBLIC MKL_ILP64
    )
    target_link_libraries(meomp-mm-blas
        PUBLIC meomp
        PRIVATE mefdn-options
        PRIVATE medsm2-exec-options
        PUBLIC ${BLAS_LIBRARIES}
    )

    add_executable(omp-mm-blas
        example/mm.cpp
    )
    target_compile_options(omp-mm-blas
        PRIVATE -fopenmp
    )
    target_compile_definitions(omp-mm-blas
        PUBLIC BENCH_USE_BLAS
        PUBLIC MKL_ILP64
    )
    target_link_libraries(omp-mm-blas
        PUBLIC -fopenmp
        PUBLIC ${BLAS_LIBRARIES}
    )
    set_target_properties(omp-mm-blas PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON)

    add_executable(seq-mm-blas
        example/mm.cpp
    )
    target_compile_definitions(seq-mm-blas
        PUBLIC BENCH_USE_BLAS
        PUBLIC MKL_ILP64
    )
    target_link_libraries(seq-mm-blas
        PUBLIC ${BLAS_LIBRARIES}
    )
    set_target_properties(seq-mm-blas PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON)
endif()


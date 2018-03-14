cmake_minimum_required(VERSION 3.0)

project(meomp)

add_library(meomp STATIC
    ./src/meomp.cpp
)
target_include_directories(meomp
    PUBLIC ./include
)
target_link_libraries(meomp
    PRIVATE medsm2
    PRIVATE mefdn-options
    PRIVATE mectx
)

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


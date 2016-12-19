cmake_minimum_required(VERSION 3.0)

project(mgdsm)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

add_library(mgdsm-header INTERFACE)
target_include_directories(mgdsm-header
    INTERFACE ./include
)
target_link_libraries(mgdsm-header
    INTERFACE mgbase-header
)

add_library(mgdsm
    ./src/sigbus_catcher.cpp
    ./src/sigsegv_catcher.cpp
    ./src/aliasing_mapped_region.cpp
    ./src/page_fault_upgrader.cpp
    ./src/dsm_space.cpp
)
target_include_directories(mgdsm
    PRIVATE ./src
)
target_link_libraries(mgdsm
    mgdsm-header
    mgcom
    mgbase-options
    mgult
)

file(GLOB example_files example/*.cpp)
foreach(example_path IN LISTS example_files)
    get_filename_component(example_name ${example_path} NAME_WE)
    add_executable(mgdsm-${example_name}
        ${example_path}
    )
    target_link_libraries(mgdsm-${example_name} mgdsm)
endforeach()


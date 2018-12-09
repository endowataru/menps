cmake_minimum_required(VERSION 3.0)

project(medsm)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

add_library(medsm
    ./src/sigbus_catcher.cpp
    ./src/sigsegv_catcher.cpp
    ./src/page_fault_upgrader.cpp
    ./src/dsm_space.cpp
)
target_include_directories(medsm
    PUBLIC ./include
    PRIVATE ./src
)
target_link_libraries(medsm
    mecom
    mefdn-options
)
if(NOT APPLE)
    # BSD's ld is not supported.
    target_link_libraries(medsm
        -T ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld
    )
endif()
install(TARGETS medsm
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Used for shm_open()
# See also: https://bugs.freedesktop.org/show_bug.cgi?id=94096
find_library(LIBRT rt)
if(LIBRT)
    target_link_libraries(medsm ${LIBRT})
endif()

file(GLOB example_files example/*.cpp)
foreach(example_path IN LISTS example_files)
    get_filename_component(example_name ${example_path} NAME_WE)
    add_executable(medsm-${example_name}
        ${example_path}
    )
    target_link_libraries(medsm-${example_name}
        medsm
    )
endforeach()


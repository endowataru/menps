cmake_minimum_required(VERSION 3.0)

project(mgth CXX)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

# Options
#option(MGTH_AVOID_REMOTE_ATOMIC "Avoid using remote atomics" ON)

# Show messages invoking CMake
message(STATUS "Generate mgth/config.h ...")
#message(STATUS "MGTH_AVOID_REMOTE_ATOMIC=${MGTH_AVOID_REMOTE_ATOMIC}")

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/mgth/config.h)

add_library(mgth
    ./src/alternate_signal_stack.cpp
    ./src/dist_scheduler.cpp
    ./src/global_ult_desc_pool.cpp
    ./src/mgth.cpp
    ./src/disable_aslr.cpp
    ./src/c_interface.cpp
    ./src/misc.cpp
)
target_include_directories(mgth
    PUBLIC ./include
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(mgth
    PUBLIC mgdsm
    PRIVATE mgbase-options
)
install(TARGETS mgth
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/
    DESTINATION include
)
# TODO: install config.h

add_library(mgth-start STATIC
    ./src/entrypoint.cpp
)
target_link_libraries(mgth-start
    PUBLIC mgth
    PRIVATE mgbase-options
)
install(TARGETS mgth-start
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

#add_executable(mgth-unittest
#    unittest/unittest.cpp
#    unittest/fcontext.cpp
#)
#target_link_libraries(mgth-unittest mgth googletest)

file(GLOB example_files example/*.cpp example/*.c)
foreach(example_path IN LISTS example_files)
    get_filename_component(example_name ${example_path} NAME_WE)
    add_executable(mgth-${example_name}
        ${example_path}
    )
    target_link_libraries(mgth-${example_name}
        mgth
        mgth-start
    )
endforeach()


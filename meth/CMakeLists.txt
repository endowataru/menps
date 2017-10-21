cmake_minimum_required(VERSION 3.0)

project(meth CXX)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

# Options
#option(METH_AVOID_REMOTE_ATOMIC "Avoid using remote atomics" ON)

# Show messages invoking CMake
message(STATUS "Generate menps/meth/config.h ...")
#message(STATUS "METH_AVOID_REMOTE_ATOMIC=${METH_AVOID_REMOTE_ATOMIC}")

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/menps/meth/config.h)

add_library(meth
    ./src/alternate_signal_stack.cpp
    ./src/dist_scheduler.cpp
    ./src/global_ult_desc_pool.cpp
    ./src/meth.cpp
    ./src/disable_aslr.cpp
    ./src/c_interface.cpp
    ./src/misc.cpp
)
target_include_directories(meth
    PUBLIC ./include
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(meth
    PUBLIC medsm
    PRIVATE mefdn-options
)
install(TARGETS meth
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/
    DESTINATION include
)
# TODO: install config.h

add_library(meth-start STATIC
    ./src/entrypoint.cpp
)
target_link_libraries(meth-start
    PUBLIC meth
    PRIVATE mefdn-options
)
install(TARGETS meth-start
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

#add_executable(meth-unittest
#    unittest/unittest.cpp
#    unittest/fcontext.cpp
#)
#target_link_libraries(meth-unittest meth googletest)

#file(GLOB example_files example/*.cpp example/*.c)
file(GLOB example_files example/*.cpp)
foreach(example_path IN LISTS example_files)
    get_filename_component(example_name ${example_path} NAME_WE)
    add_executable(meth-${example_name}
        ${example_path}
    )
    target_link_libraries(meth-${example_name}
        meth
        meth-start
    )
endforeach()


cmake_minimum_required(VERSION 3.0)

project(mgth CXX)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

add_library(mgth STATIC
    ./src/alternate_signal_stack.cpp
    ./src/dist_scheduler.cpp
    ./src/global_ult_desc_pool.cpp
    ./src/entrypoint.cpp
    ./src/disable_aslr.cpp
    ./src/c_interface.cpp
)

target_include_directories(mgth
    PUBLIC ./include
)
target_link_libraries(mgth
    mgbase-options
    mgbase
    mgdsm
    mgult
)
install(TARGETS mgth
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/
    DESTINATION include
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
    target_link_libraries(mgth-${example_name} mgth)
endforeach()


cmake_minimum_required(VERSION 3.0)

project(mgth CXX)

add_library(mgth STATIC
    ./src/dist_scheduler.cpp
    ./src/entrypoint.cpp
    ./src/global_ult_desc_pool.cpp
    #./src/disable_aslr.cpp
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

#add_executable(mgth-unittest
#    unittest/unittest.cpp
#    unittest/fcontext.cpp
#)
#target_link_libraries(mgth-unittest mgth googletest)

file(GLOB example_files example/*.cpp)
foreach(example_path IN LISTS example_files)
    get_filename_component(example_name ${example_path} NAME_WE)
    add_executable(mgth-${example_name}
        ${example_path}
    )
    target_link_libraries(mgth-${example_name} mgth)
endforeach()


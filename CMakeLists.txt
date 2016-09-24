cmake_minimum_required(VERSION 3.0)

project(mgult CXX ASM)

add_library(mgult-header INTERFACE)
target_include_directories(mgult-header
    INTERFACE ./include
)
target_link_libraries(mgult-header
    INTERFACE mgbase-header
    mgcom-header
)

#enable_language(ASM)

add_library(mgult
    ./src/fcontext/jump_x86_64_sysv_macho_gas.S
    ./src/fcontext/make_x86_64_sysv_macho_gas.S
    ./src/fcontext/ontop_x86_64_sysv_macho_gas.S
    ./src/worker.cpp
)

#set_source_files_properties(
#    ./src/fcontext/fcontext.x86_64.S PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
target_include_directories(mgult
    PUBLIC ./include
)
target_link_libraries(mgult mgult-header mgbase-options mgbase)

add_executable(mgult-unittest
    unittest/unittest.cpp
    unittest/fcontext.cpp
)
target_link_libraries(mgult-unittest mgult googletest)


file(GLOB example_files example/*.cpp)
foreach(example_path IN LISTS example_files)
    get_filename_component(example_name ${example_path} NAME_WE)
    add_executable(mgult-${example_name}
        ${example_path}
    )
    target_link_libraries(mgult-${example_name} mgult)
endforeach()


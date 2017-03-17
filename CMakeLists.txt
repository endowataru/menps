cmake_minimum_required(VERSION 3.0)

project(mgult CXX ASM)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

add_library(mgult
    $<$<PLATFORM_ID:Linux>:
    ./src/fcontext/jump_x86_64_sysv_elf_gas.S
    ./src/fcontext/make_x86_64_sysv_elf_gas.S
    ./src/fcontext/ontop_x86_64_sysv_elf_gas.S
    >
    $<$<PLATFORM_ID:Darwin>:
    ./src/fcontext/jump_x86_64_sysv_macho_gas.S
    ./src/fcontext/make_x86_64_sysv_macho_gas.S
    ./src/fcontext/ontop_x86_64_sysv_macho_gas.S
    >
    
    ./src/sm/sm_scheduler.cpp
    ./src/sm/initializer.cpp
    ./src/scheduler_initializer.cpp
)
target_include_directories(mgult
    PUBLIC ./include
)
target_link_libraries(mgult
    PUBLIC mgctx
    PRIVATE mgbase-options
)
install(TARGETS mgult
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Unit tests

add_executable(mgult-unittest
    unittest/unittest.cpp
    unittest/fcontext.cpp
)
target_link_libraries(mgult-unittest
    mgult
    mgbase-options
    googletest
)

file(GLOB example_files example/*.cpp)
foreach(example_path IN LISTS example_files)
    get_filename_component(example_name ${example_path} NAME_WE)
    add_executable(mgult-${example_name}
        ${example_path}
    )
    target_link_libraries(mgult-${example_name}
        mgult
        mgbase-options
    )
endforeach()


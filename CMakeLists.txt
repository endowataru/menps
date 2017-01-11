cmake_minimum_required(VERSION 3.0)

project(mgctx CXX ASM)

add_library(mgctx
    src/empty.cpp
    
    $<$<PLATFORM_ID:Linux>:
    ./src/arch/sysv_x86_64_elf.S
    >
    $<$<PLATFORM_ID:Darwin>:
    ./src/arch/sysv_x86_64_macho.S
    >
)
target_include_directories(mgctx
    PUBLIC ./include
)
target_link_libraries(mgctx
    mgbase
    mgbase-options
)

add_executable(mgctx-unittest
    unittest/unittest.cpp
    unittest/context.cpp
)
target_link_libraries(mgctx-unittest
    mgctx
    googletest
)


cmake_minimum_required(VERSION 3.0)

project(mectx CXX ASM)

add_library(mectx
    src/empty.cpp
    
    $<$<PLATFORM_ID:Linux>:
    ./src/arch/sysv_x86_64_elf.S
    >
    $<$<PLATFORM_ID:Darwin>:
    ./src/arch/sysv_x86_64_macho.S
    >
)
target_include_directories(mectx
    PUBLIC ./include
)
target_link_libraries(mectx
    PUBLIC mefdn
    PRIVATE mefdn-options
)
install(TARGETS mectx
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)


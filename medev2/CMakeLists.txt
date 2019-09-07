cmake_minimum_required(VERSION 3.0)

project(medev2)

# Show messages when invoking CMake
message(STATUS "Generate menps/medev2/config.h ...")

set(MEDEV2_PROF_ASPECT "DUMMY" CACHE STRING "Prof aspect for medev2")

if(APPLE)
    # UCX is not supported on macOS.
    option(MEDEV2_DEVICE_UCX_ENABLED "Enable UCX" FALSE)
else()
    option(MEDEV2_DEVICE_UCX_ENABLED "Enable UCX" TRUE)
endif()

option(MEDEV2_SERIALIZE_MPI_CALLS "Serialize MPI calls" FALSE)

option(MEDEV2_AVOID_SWITCH_IN_SIGNAL "Avoid using context switch in signal handler" FALSE)

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/menps/medev2/config.h)

if(${MEDEV2_DEVICE_UCX_ENABLED})
    include(cmake/ucx.cmake)
endif()

add_library(medev2 INTERFACE)

target_include_directories(medev2
    INTERFACE ./include
    INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(medev2
    INTERFACE mefdn
    INTERFACE $<$<BOOL:${MEDEV2_DEVICE_UCX_ENABLED}>:ucp>
    INTERFACE $<$<BOOL:${MEDEV2_DEVICE_UCX_ENABLED}>:uct>
    INTERFACE $<$<BOOL:${MEDEV2_DEVICE_UCX_ENABLED}>:ucs>
)
install(DIRECTORY include/ DESTINATION include)

if(${MEDEV2_DEVICE_UCX_ENABLED})
    add_executable(medev2_ucp_test
        ./examples/ucp_test.cpp
    )
    target_link_libraries(medev2_ucp_test
        PUBLIC medev2
        PRIVATE mefdn-options
    )

    add_executable(medev2_uct_test
        ./examples/uct_test.cpp
    )
    target_link_libraries(medev2_uct_test
        PUBLIC medev2
        PRIVATE mefdn-options
    )
endif()


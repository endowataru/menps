cmake_minimum_required(VERSION 3.0)

project(meult CXX ASM)

# Do not link shared libraries again
set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

# Add RPATH in installation
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Options
option(MEULT_ENABLE_MTH "Build MassiveThreads" ON)
option(MEULT_ENABLE_MTH_WORKER_CACHE "Enable caching for worker numbers" ON)
option(MEULT_ENABLE_MTH_UNCOND_ENTER "Enable myth_uncond_enter" ON)
option(MEULT_ENABLE_BALANCED_POOL "Use balanced memory pool" ON)
set(MEULT_BALANCED_POOL_MAX_ENTRIES 128 CACHE STRING "Max of entries pooled for remote workers")
option(MEULT_QD_USE_UNCOND_ENTER_FOR_TRANSFER "Use myth_uncond_enter for lock transfers in QD" ON)
option(MEULT_QD_ENABLE_ATOMIC_COUNT "Enable counting in QD for debugging" OFF)
set(MEULT_EXECUTOR_LIFETIME_CYCLES "0 " CACHE STRING "Run executors during at least this time")

# Show messages invoking CMake
message(STATUS "Generate menps/meult/config.h ...")
message(STATUS "MEULT_ENABLE_MTH=${MEULT_ENABLE_MTH}")

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/menps/meult/config.h)

add_library(meult
    ./src/sm/sm_scheduler.cpp
    ./src/sm/ult_desc_pool.cpp
    ./src/sm/initializer.cpp
    ./src/scheduler_initializer.cpp
    ./src/constants.cpp
)
target_include_directories(meult
    PUBLIC ./include
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(meult
    PUBLIC mectx
    PRIVATE mefdn-options
)
install(TARGETS meult
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)

# MassiveThreads support
if (${MEULT_ENABLE_MTH})
    #include(cmake/massivethreads.cmake)
    # massivethreads is now build on cmpth
    
    target_link_libraries(meult
        PUBLIC massivethreads
    )
endif()


# Unit tests

#add_executable(meult-unittest
#    unittest/unittest.cpp
#    unittest/qdlock.cpp
#)
#target_link_libraries(meult-unittest
#    meult
#    mefdn-options
#    googletest
#)

file(GLOB example_files example/*.cpp)
foreach(example_path IN LISTS example_files)
    get_filename_component(example_name ${example_path} NAME_WE)
    add_executable(meult-${example_name}
        ${example_path}
    )
    target_link_libraries(meult-${example_name}
        meult
        mefdn-options
    )
endforeach()


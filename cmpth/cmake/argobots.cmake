cmake_minimum_required(VERSION 3.1)

include(ExternalProject)

set(CMPTH_ARGOBOTS_CC ${CMAKE_C_COMPILER})
set(CMPTH_ARGOBOTS_CFLAGS ${CMPTH_GLOBAL_CFLAGS} -Wall)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMPTH_ARGOBOTS_CONFIGURE_FLAGS
        --enable-debug=yes
        --enable-fast=none
        --enable-tls-model=initial-exec
        --disable-preserve-fpu
    )
elseif (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    set(CMPTH_ARGOBOTS_CONFIGURE_FLAGS
        --enable-debug=yes
        --enable-fast=O2
        --enable-tls-model=initial-exec
        --disable-preserve-fpu
    )
else()
    set(CMPTH_ARGOBOTS_CONFIGURE_FLAGS
        --enable-debug=none
        --enable-fast=O3,ndebug
        --enable-tls-model=initial-exec
        --disable-preserve-fpu
    )
endif()

# Convert from list to string
string(REPLACE ";" " " CMPTH_ARGOBOTS_CFLAGS "${CMPTH_ARGOBOTS_CFLAGS}")

configure_file(cmake/argobots-configure.sh.in argobots-configure.sh @ONLY)

ExternalProject_Add(Argobots
    URL ${PROJECT_SOURCE_DIR}/external/argobots
    #SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/argobots

    CONFIGURE_COMMAND /bin/sh ${CMAKE_CURRENT_BINARY_DIR}/argobots-configure.sh
        <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
            ${CMPTH_ARGOBOTS_CONFIGURE_FLAGS}

    BUILD_COMMAND $(MAKE) V=1
)

ExternalProject_Add_Step(Argobots autogen
    COMMAND ./autogen.sh
    DEPENDEES patch
    DEPENDERS configure
    WORKING_DIRECTORY <SOURCE_DIR>
)

ExternalProject_Get_Property(Argobots source_dir)
ExternalProject_Get_Property(Argobots install_dir)
ExternalProject_Get_Property(Argobots binary_dir)

add_library(argobots SHARED IMPORTED GLOBAL)
add_dependencies(argobots Argobots)

set_property(
    TARGET argobots
    PROPERTY IMPORTED_LOCATION
    ${install_dir}/lib/libabt.so
)

# Make a directory for include path if it doesn't exist
# Reported as a bug of CMake:
# https://cmake.org/Bug/view.php?id=15052
file(MAKE_DIRECTORY ${install_dir}/include)

set_property(
    TARGET argobots
    APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${install_dir}/include
)


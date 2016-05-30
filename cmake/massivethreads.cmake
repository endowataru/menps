cmake_minimum_required(VERSION 3.0)

include(ExternalProject)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(MGBASE_MASSIVETHREADS_CFLAGS "-O0 -g")
endif()

configure_file(cmake/massivethreads-configure.sh.in massivethreads-configure.sh @ONLY)

ExternalProject_Add(MassiveThreads
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/massivethreads
    #GIT_REPOSITORY https://github.com/massivethreads/massivethreads
    CONFIGURE_COMMAND /bin/sh ${CMAKE_CURRENT_BINARY_DIR}/massivethreads-configure.sh
        <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --enable-malloc-wrapper=no)

ExternalProject_Get_Property(MassiveThreads source_dir)
ExternalProject_Get_Property(MassiveThreads install_dir)

add_library(massivethreads SHARED IMPORTED GLOBAL)
add_dependencies(massivethreads MassiveThreads)

set_property(
    TARGET massivethreads
    PROPERTY IMPORTED_LOCATION
    ${install_dir}/lib/libmyth-native.so)

set_property(
    TARGET massivethreads
    APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${install_dir}/include)

set_property(
    TARGET massivethreads
    APPEND PROPERTY INTERFACE_LINK_LIBRARIES
    dl
    pthread)


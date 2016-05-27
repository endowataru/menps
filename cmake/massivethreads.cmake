cmake_minimum_required(VERSION 3.0)

include(ExternalProject)

ExternalProject_Add(MassiveThreads
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/massivethreads
    #GIT_REPOSITORY https://github.com/massivethreads/massivethreads
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>)

ExternalProject_Get_Property(MassiveThreads source_dir)
ExternalProject_Get_Property(MassiveThreads install_dir)

add_library(massivethreads STATIC IMPORTED GLOBAL)
add_dependencies(massivethreads MassiveThreads)

set_property(
    TARGET massivethreads
    PROPERTY IMPORTED_LOCATION
    ${install_dir}/lib/libmyth.a)

set_property(
    TARGET massivethreads
    APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${source_dir}/include)

set_property(
    TARGET massivethreads
    APPEND PROPERTY INTERFACE_LINK_LIBRARIES
    dl
    pthread
    )


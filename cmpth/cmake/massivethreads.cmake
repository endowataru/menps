cmake_minimum_required(VERSION 3.0)

include(ExternalProject)

set(CMPTH_MASSIVETHREADS_CC ${CMAKE_C_COMPILER})

set(CMPTH_MASSIVETHREADS_CFLAGS ${CMPTH_GLOBAL_CFLAGS} -Wall)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    list(APPEND CMPTH_MASSIVETHREADS_CFLAGS -O0 -g)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    list(APPEND CMPTH_MASSIVETHREADS_CFLAGS -O3 -g -DNDEBUG)
else()
    list(APPEND CMPTH_MASSIVETHREADS_CFLAGS -O3 -DNDEBUG)
endif()

# Convert from list to string
string(REPLACE ";" " " CMPTH_MASSIVETHREADS_CFLAGS "${CMPTH_MASSIVETHREADS_CFLAGS}")

configure_file(cmake/massivethreads-configure.sh.in massivethreads-configure.sh @ONLY)

ExternalProject_Add(MassiveThreads
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/massivethreads
    #GIT_REPOSITORY https://github.com/massivethreads/massivethreads
    CONFIGURE_COMMAND /bin/sh ${CMAKE_CURRENT_BINARY_DIR}/massivethreads-configure.sh
        <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
        --enable-myth-ld=no)
    # TODO: myth-ld produces this error:
    #   gcc: error: @myth-ld.opts: No such file or directory

ExternalProject_Get_Property(MassiveThreads source_dir)
ExternalProject_Get_Property(MassiveThreads install_dir)

add_library(massivethreads SHARED IMPORTED GLOBAL)
add_dependencies(massivethreads MassiveThreads)

set_property(
    TARGET massivethreads
    PROPERTY IMPORTED_LOCATION
    ${install_dir}/lib/libmyth${CMAKE_SHARED_LIBRARY_SUFFIX})

# Make a directory for include path if it doesn't exist
# Reported as a bug of CMake:
# https://cmake.org/Bug/view.php?id=15052
file(MAKE_DIRECTORY ${install_dir}/include)

set_property(
    TARGET massivethreads
    APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${install_dir}/include)

set_property(
    TARGET massivethreads
    APPEND PROPERTY INTERFACE_LINK_LIBRARIES
    dl
    pthread)


install(DIRECTORY ${install_dir}/bin/       DESTINATION bin)
install(DIRECTORY ${install_dir}/include/   DESTINATION include)
install(DIRECTORY ${install_dir}/lib/       DESTINATION lib)


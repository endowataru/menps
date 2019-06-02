cmake_minimum_required(VERSION 3.0)

include(ExternalProject)

# Note: UCX's configure adds "-g" by default (BASE_CFLAGS)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(MEDEV2_UCX_CONFIGURE_FLAGS
        --enable-compiler-opt=none # == "-O0"
        --enable-debug
        
        #--enable-gtest             # Disabled to shorten build time
        #--with-valgrind            # TODO: for ReedBush
        #--enable-profiling         # Disabled
        #--enable-frame-pointer     # TODO
        #--enable-stats             # Disabled
        #--enable-memtrack          # Disabled
        #--enable-fault-injection   # Disabled; seemed meaningless
        --enable-debug-data
        #--enable-mt                # Disabled
    )
elseif (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    set(MEDEV2_UCX_CONFIGURE_FLAGS
        --enable-compiler-opt
        --disable-debug # TODO
        
        --enable-optimizations
        --disable-logging
        --disable-debug
        --disable-assertions
        --disable-params-check
    )
else()
    set(MEDEV2_UCX_CONFIGURE_FLAGS
        --enable-compiler-opt
        --disable-debug
        
        --enable-optimizations
        --disable-logging
        --disable-debug
        --disable-assertions
        --disable-params-check
    )
endif()

set(MEDEV2_UCX_CFLAGS ${MEFDN_GLOBAL_CFLAGS} -Wall)

# Convert from list to string
string(REPLACE ";" " " MEDEV2_UCX_CFLAGS "${MEDEV2_UCX_CFLAGS}")

list(APPEND MEDEV2_UCX_CONFIGURE_FLAGS --without-cuda)
# TODO: Including <cuda.h> fails on ReedBush

configure_file(cmake/ucx-configure.sh.in ucx-configure.sh @ONLY)

ExternalProject_Add(UCX
    URL ${PROJECT_SOURCE_DIR}/external/ucx
    
    # Why does it require OpenSHMEM???
    #PATCH_COMMAND sed -i ""
    #    "s/^noinst_PROGRAMS       = test_memhooks shmem_pingpong/noinst_PROGRAMS       = test_memhooks/"
    #    <SOURCE_DIR>/test/mpi/Makefile.am
    
    CONFIGURE_COMMAND /bin/sh
        ${CMAKE_CURRENT_BINARY_DIR}/ucx-configure.sh
        <SOURCE_DIR>/configure
        --prefix=<INSTALL_DIR>
        ${MEDEV2_UCX_CONFIGURE_FLAGS}
    
    BUILD_COMMAND $(MAKE) V=1
)

ExternalProject_Add_Step(UCX autogen
    COMMAND ./autogen.sh
    DEPENDEES patch
    DEPENDERS configure
    WORKING_DIRECTORY <SOURCE_DIR>
)

ExternalProject_Get_Property(UCX source_dir)
ExternalProject_Get_Property(UCX install_dir)
ExternalProject_Get_Property(UCX binary_dir)

add_library(ucp SHARED IMPORTED GLOBAL)
add_dependencies(ucp UCX)

add_library(uct SHARED IMPORTED GLOBAL)
add_dependencies(uct UCX)

add_library(ucs SHARED IMPORTED GLOBAL)
add_dependencies(ucs UCX)

set_property(
    TARGET ucp
    PROPERTY IMPORTED_LOCATION
    ${install_dir}/lib/libucp.so
)

set_property(
    TARGET uct
    PROPERTY IMPORTED_LOCATION
    ${install_dir}/lib/libuct.so
)

set_property(
    TARGET ucs
    PROPERTY IMPORTED_LOCATION
    ${install_dir}/lib/libucs.so
)

# Make a directory for include path if it doesn't exist
# Reported as a bug of CMake:
# https://cmake.org/Bug/view.php?id=15052
file(MAKE_DIRECTORY ${install_dir}/include)

set_property(
    TARGET ucp
    APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${install_dir}/include)

set_property(
    TARGET uct
    APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${install_dir}/include)

set_property(
    TARGET ucs
    APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${install_dir}/include)

#set_property(
#    TARGET ucx
#    APPEND PROPERTY INTERFACE_LINK_LIBRARIES
#    dl
#    pthread
#    )

add_library(ucx-src INTERFACE)

target_include_directories(ucx-src INTERFACE
    ${source_dir}/src
    ${binary_dir}
)

# Make a directory for include path if it doesn't exist
# Reported as a bug of CMake:
# https://cmake.org/Bug/view.php?id=15052
file(MAKE_DIRECTORY ${source_dir}/include)


install(DIRECTORY ${install_dir}/bin/       DESTINATION bin)
install(DIRECTORY ${install_dir}/include/   DESTINATION include)
install(DIRECTORY ${install_dir}/lib/       DESTINATION lib)
install(DIRECTORY ${install_dir}/share/     DESTINATION share)


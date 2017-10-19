cmake_minimum_required(VERSION 3.0)

include(ExternalProject)

ExternalProject_Add(UCX
    URL ${PROJECT_SOURCE_DIR}/external/ucx
    
    # Why does it require OpenSHMEM???
    #PATCH_COMMAND sed -i ""
    #    "s/^noinst_PROGRAMS       = test_memhooks shmem_pingpong/noinst_PROGRAMS       = test_memhooks/"
    #    <SOURCE_DIR>/test/mpi/Makefile.am
    
    CONFIGURE_COMMAND <SOURCE_DIR>/contrib/configure-release --prefix=<INSTALL_DIR>
    
    #--with-mpi
    #    --disable-numa
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


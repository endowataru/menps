cmake_minimum_required(VERSION 3.0)

include(ExternalProject)

ExternalProject_Add(UCX
    URL ${PROJECT_SOURCE_DIR}/external/ucx
    
    # Why does it require OpenSHMEM???
    PATCH_COMMAND sed -i
        "s/^noinst_PROGRAMS       = test_memhooks shmem_pingpong/noinst_PROGRAMS       = test_memhooks/"
        <SOURCE_DIR>/test/mpi/Makefile.am
    
    CONFIGURE_COMMAND <SOURCE_DIR>/contrib/configure-release --prefix=<INSTALL_DIR> --with-mpi
    )

ExternalProject_Add_Step(UCX autogen
    COMMAND ./autogen.sh
    DEPENDEES patch
    DEPENDERS configure
    WORKING_DIRECTORY <SOURCE_DIR>
    )

#ExternalProject_Get_Property(UCX source_dir)
ExternalProject_Get_Property(UCX install_dir)

add_library(ucx SHARED IMPORTED GLOBAL)
add_dependencies(ucx MassiveThreads)

#set_property(
#    TARGET ucx
#    PROPERTY IMPORTED_LOCATION
#    #${install_dir}/lib/libmyth-native.so)

# Make a directory for include path if it doesn't exist
# Reported as a bug of CMake:
# https://cmake.org/Bug/view.php?id=15052
#file(MAKE_DIRECTORY ${install_dir}/include)

set_property(
    TARGET ucx
    APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${install_dir}/include)

#set_property(
#    TARGET ucx
#    APPEND PROPERTY INTERFACE_LINK_LIBRARIES
#    dl
#    pthread
#    )


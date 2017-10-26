cmake_minimum_required(VERSION 3.0)

if(${MEDEV_DEVICE_UCX_ENABLED})

project(meucp CXX)

add_library(meucp-options INTERFACE)
target_link_libraries(meucp-options INTERFACE
    # grep -E "^(\w)+ ucp_(\w)+\(" ucp.h | sed -e "s/(/ /" | awk '{ print $2 }'
    
    #-Wl,--wrap=ucp_config_read
    #-Wl,--wrap=ucp_config_release
    #-Wl,--wrap=ucp_config_modify
    #-Wl,--wrap=ucp_config_print
    #-Wl,--wrap=ucp_get_version
    #-Wl,--wrap=ucp_init_version
    #-Wl,--wrap=ucp_cleanup
    #-Wl,--wrap=ucp_context_query
    #-Wl,--wrap=ucp_context_print_info
    -Wl,--wrap=ucp_worker_create
    -Wl,--wrap=ucp_worker_destroy
    -Wl,--wrap=ucp_worker_query
    -Wl,--wrap=ucp_worker_print_info
    -Wl,--wrap=ucp_worker_get_address
    -Wl,--wrap=ucp_worker_release_address
    -Wl,--wrap=ucp_worker_progress
    -Wl,--wrap=ucp_worker_get_efd
    -Wl,--wrap=ucp_worker_wait
    -Wl,--wrap=ucp_worker_wait_mem
    -Wl,--wrap=ucp_worker_arm
    -Wl,--wrap=ucp_worker_signal
    -Wl,--wrap=ucp_ep_create
    -Wl,--wrap=ucp_ep_destroy # deprecated
    -Wl,--wrap=ucp_disconnect_nb
    -Wl,--wrap=ucp_ep_print_info
    -Wl,--wrap=ucp_ep_flush
    #-Wl,--wrap=ucp_mem_map
    #-Wl,--wrap=ucp_mem_unmap
    #-Wl,--wrap=ucp_mem_query
    #-Wl,--wrap=ucp_mem_advise
    #-Wl,--wrap=ucp_rkey_pack
    #-Wl,--wrap=ucp_rkey_buffer_release
    -Wl,--wrap=ucp_ep_rkey_unpack
    #-Wl,--wrap=ucp_rkey_destroy
    -Wl,--wrap=ucp_tag_send_nb
    -Wl,--wrap=ucp_tag_send_sync_nb
    -Wl,--wrap=ucp_tag_recv_nb
    -Wl,--wrap=ucp_tag_recv_nbr
    -Wl,--wrap=ucp_tag_probe_nb
    -Wl,--wrap=ucp_tag_msg_recv_nb
    -Wl,--wrap=ucp_put
    -Wl,--wrap=ucp_put_nbi
    -Wl,--wrap=ucp_get
    -Wl,--wrap=ucp_get_nbi
    -Wl,--wrap=ucp_atomic_add32
    -Wl,--wrap=ucp_atomic_add64
    -Wl,--wrap=ucp_atomic_fadd32
    -Wl,--wrap=ucp_atomic_fadd64
    -Wl,--wrap=ucp_atomic_swap32
    -Wl,--wrap=ucp_atomic_swap64
    -Wl,--wrap=ucp_atomic_cswap32
    -Wl,--wrap=ucp_atomic_cswap64
    -Wl,--wrap=ucp_atomic_post
    -Wl,--wrap=ucp_atomic_fetch_nb
    -Wl,--wrap=ucp_request_test
    -Wl,--wrap=ucp_request_cancel
    -Wl,--wrap=ucp_request_free
    -Wl,--wrap=ucp_dt_create_generic
    -Wl,--wrap=ucp_dt_destroy
    -Wl,--wrap=ucp_worker_fence
    -Wl,--wrap=ucp_worker_flush
)

add_library(meucp
    src/worker.cpp
)
target_include_directories(meucp
    PUBLIC ./include
)
target_link_libraries(meucp
    PUBLIC medev-ucp
    PRIVATE mefdn-options
    PRIVATE meucp-options
)
install(TARGETS meucp
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)


#add_executable(meucp-ucp_test
#    ../medev/examples/ucx/ucp/ucp_test.cpp
#)
#target_link_libraries(meucp-ucp_test
#    PUBLIC meucp
#    PRIVATE mefdn-options
#    PRIVATE -Wl,--wrap=ucp_init_version
#)


add_executable(meucp-ucp_test
    ../medev/external/ucx/test/examples/ucp_hello_world.c
)
target_link_libraries(meucp-ucp_test
    PUBLIC meucp
    #PRIVATE mefdn-options # to avoid use C99
    PRIVATE meucp-options
)

add_executable(meucp-perftest
    # TODO: do not use the relative path.
    ../medev/external/ucx/src/tools/perf/libperf.c
    ../medev/external/ucx/src/tools/perf/perftest.c
    ../medev/external/ucx/src/tools/perf/ucp_tests.cc
    ../medev/external/ucx/src/tools/perf/uct_tests.cc
)
target_compile_options(meucp-perftest
    PUBLIC -DCPU_FLAGS=
    -DHAVE_CONFIG_H
)
target_link_libraries(meucp-perftest
    PUBLIC meucp
    PUBLIC ucs
    PUBLIC uct
    PUBLIC ucx-src
    #PRIVATE mefdn-options # to avoid use C99
    PRIVATE meucp-options
)

endif()



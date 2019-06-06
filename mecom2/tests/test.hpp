
#pragma once

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

TEST_CASE("Read from other RMA buffers")
{
    auto buf = g_com->make_alltoall_buffer<int>(1);
    *buf.local(0) = g_com->this_proc_id() + 100;
    
    g_rma->flush();
    
    g_coll->barrier();
    
    for (proc_id_type i = 0; i < g_com->get_num_procs(); ++i)
    {
        auto buf_i = g_rma->buf_read(i, buf.remote(i, 0), 1);
        const auto val = buf_i[0];
        REQUIRE(val == i+100);
    }
    
    g_coll->barrier();
}



#include <mgcom.hpp>

#include "../src-device/mpi3/rma.hpp"

namespace mgcom {

namespace rma {

namespace {

void test_read(void* /*cb_*/) {
    poll();
}

void start_read(void* cb_) {
    read_cb& cb = *static_cast<read_cb*>(cb_);
    
    if (try_read_async(cb.local_addr, cb.remote_addr, cb.size_in_bytes, cb.dest_proc,
        make_notifier_assign(&cb.request.state, MGBASE_STATE_FINISHED)))
    {
        mgbase::async_enter(&cb, test_read);
    }
}

}

void read_async(
    read_cb*              cb
,   const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
)
{
    cb->local_addr    = local_addr;
    cb->remote_addr   = remote_addr;
    cb->size_in_bytes = size_in_bytes;
    cb->dest_proc     = dest_proc;
    mgbase::async_enter(cb, start_read);
}

namespace {

void test_write(void* /*cb_*/) {
    poll();
}

void start_write(void* cb_) {
    write_cb& cb = *static_cast<write_cb*>(cb_);
    
    if (try_write_async(cb.local_addr, cb.remote_addr, cb.size_in_bytes, cb.dest_proc,
        make_notifier_assign(&cb.request.state, MGBASE_STATE_FINISHED)))
    {
        mgbase::async_enter(&cb, test_write);
    }
}

}

void write_async(
    write_cb*             cb
,   const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
)
{
    cb->local_addr    = local_addr;
    cb->remote_addr   = remote_addr;
    cb->size_in_bytes = size_in_bytes;
    cb->dest_proc     = dest_proc;
    mgbase::async_enter(cb, start_write);
}

namespace {

void test_compare_and_swap_64(void* /*cb_*/) {
    poll();
}

void start_compare_and_swap_64(void* cb_) {
    compare_and_swap_64_cb& cb = *static_cast<compare_and_swap_64_cb*>(cb_);
    
    if (try_compare_and_swap_64_async(cb.remote_addr, &cb.expected, &cb.desired, cb.result, cb.dest_proc,
        make_notifier_assign(&cb.request.state, MGBASE_STATE_FINISHED)))
    {
        mgbase::async_enter(&cb, test_compare_and_swap_64);
    }
}

}
void compare_and_swap_64_async(
    compare_and_swap_64_cb* cb
,   remote_address          remote_addr
,   mgbase::uint64_t        expected
,   mgbase::uint64_t        desired
,   mgbase::uint64_t*       result
,   process_id_t            dest_proc
)
{
    cb->remote_addr = remote_addr;
    cb->expected    = expected;
    cb->desired     = desired;
    cb->result      = result;
    cb->dest_proc   = dest_proc;
    mgbase::async_enter(cb, start_compare_and_swap_64);
}

}

}


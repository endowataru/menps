
#include <mgcom.hpp>

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
    cb->local_addr = local_addr;
    cb->remote_addr = remote_addr;
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
    cb->local_addr = local_addr;
    cb->remote_addr = remote_addr;
    cb->size_in_bytes = size_in_bytes;
    cb->dest_proc     = dest_proc;
    mgbase::async_enter(cb, start_write);
}

}

}


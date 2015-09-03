
#include <mgcom.hpp>

#include "../src-device/mpi3/rma.hpp"

namespace mgcom {

namespace rma {

namespace detail {

namespace {

void test_read(void* /*cb_*/) {
    poll();
}

void start_read(void* cb_) {
    read_cb& cb = *static_cast<read_cb*>(cb_);
    
    if (try_read(cb.local_addr, cb.remote_addr, cb.size_in_bytes, cb.dest_proc,
        make_notifier_assign(&cb.request.state, MGBASE_STATE_FINISHED)))
    {
        mgbase::async_enter(&cb, test_read);
    }
}

}

void read_nb(read_cb* cb)
{
    mgbase::async_enter(cb, start_read);
}

namespace {

void test_write(void* /*cb_*/) {
    poll();
}

void start_write(void* cb_) {
    write_cb& cb = *static_cast<write_cb*>(cb_);
    
    if (try_write(cb.local_addr, cb.remote_addr, cb.size_in_bytes, cb.dest_proc,
        make_notifier_assign(&cb.request.state, MGBASE_STATE_FINISHED)))
    {
        mgbase::async_enter(&cb, test_write);
    }
}

}

void write_nb(write_cb* cb)
{
    mgbase::async_enter(cb, start_write);
}

namespace {

void test_compare_and_swap_64(void* /*cb_*/) {
    poll();
}

void start_compare_and_swap_64(void* cb_) {
    compare_and_swap_64_cb& cb = *static_cast<compare_and_swap_64_cb*>(cb_);
    
    if (try_compare_and_swap_64(cb.remote_addr, &cb.expected, &cb.desired, cb.result, cb.dest_proc,
        make_notifier_assign(&cb.request.state, MGBASE_STATE_FINISHED)))
    {
        mgbase::async_enter(&cb, test_compare_and_swap_64);
    }
}

}

void compare_and_swap_64_nb(compare_and_swap_64_cb* cb)
{
    mgbase::async_enter(cb, start_compare_and_swap_64);
}

}

}

}



#include <mgcom.hpp>

#include "impl.hpp"

#include "../src-device/mpi3/rma.hpp"

namespace mgcom {

namespace rma {

namespace detail {

namespace {

void test_read(read_cb& /*cb*/) {
    poll();
}

void start_read(read_cb& cb) {
    if (try_read(cb.local_addr, cb.remote_addr, cb.size_in_bytes, cb.dest_proc,
        make_notifier_finished(cb)))
    {
        mgbase::control::enter<read_cb, test_read>(cb);
    }
}

}

void read_nb(read_cb& cb)
{
    mgbase::control::start<read_cb, start_read>(cb);
}

namespace {

void test_write(write_cb& /*cb*/) {
    poll();
}

void start_write(write_cb& cb) {
    if (try_write(cb.local_addr, cb.remote_addr, cb.size_in_bytes,
        cb.dest_proc, make_notifier_finished(cb)))
    {
        mgbase::control::enter<write_cb, test_write>(cb);
    }
}

}

void write_nb(write_cb& cb)
{
    mgbase::control::start<write_cb, start_write>(cb);
}

namespace {

void test_compare_and_swap_64(compare_and_swap_64_cb& /*cb*/) {
    poll();
}

void start_compare_and_swap_64(compare_and_swap_64_cb& cb) {
    if (try_compare_and_swap_64(cb.remote_addr, &cb.expected, &cb.desired,
        cb.result, cb.dest_proc, make_notifier_finished(cb)))
    {
        mgbase::control::enter<compare_and_swap_64_cb, test_compare_and_swap_64>(cb);
    }
}

}

void compare_and_swap_64_nb(compare_and_swap_64_cb& cb)
{
    mgbase::control::start<compare_and_swap_64_cb, start_compare_and_swap_64>(cb);
}

}

}

}


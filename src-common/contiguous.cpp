
#include "contiguous.ipp"

namespace mgcom {

namespace rma {

namespace detail {

void read_nb(read_cb& cb) {
    read_handlers::start(cb);
}

void write_nb(write_cb& cb) {
    write_handlers::start(cb);
}

void atomic_write_64_nb(atomic_write_64_cb& cb) {
    atomic_write_64_handlers::start(cb);
}

void atomic_read_64_nb(atomic_read_64_cb& cb) {
    atomic_read_64_handlers::start(cb);
}

void compare_and_swap_64_nb(compare_and_swap_64_cb& cb) {
    compare_and_swap_64_handlers::start(cb);
}

void fetch_and_add_64_nb(fetch_and_op_64_cb& cb) {
    fetch_and_add_64_handlers::start(cb);
}

}

}

}



#include "contiguous.ipp"

namespace mgcom {

namespace rma {

namespace detail {

void remote_read_nb(remote_read_cb& cb) {
    remote_read_handlers::start(cb);
}

void remote_write_nb(remote_write_cb& cb) {
    remote_write_handlers::start(cb);
}

void remote_atomic_write_default_nb(remote_atomic_write_default_cb& cb) {
    remote_atomic_write_default_handlers::start(cb);
}

void remote_atomic_read_default_nb(remote_atomic_read_default_cb& cb) {
    remote_atomic_read_default_handlers::start(cb);
}

void remote_compare_and_swap_default_nb(remote_compare_and_swap_default_cb& cb) {
    remote_compare_and_swap_default_handlers::start(cb);
}

void remote_fetch_and_add_default_nb(remote_fetch_and_add_default_cb& cb) {
    remote_fetch_and_add_default_handlers::start(cb);
}

}

}

}



#include "try_atomic.ipp"

namespace mgcom {
namespace rma {
namespace untyped {

namespace detail {

mgbase::deferred<void> remote_atomic_write_default(remote_atomic_write_default_cb& cb) {
    return remote_atomic_write_default_handlers::start(cb);
}

mgbase::deferred<void> remote_atomic_read_default(remote_atomic_read_default_cb& cb) {
    return remote_atomic_read_default_handlers::start(cb);
}

mgbase::deferred<void> remote_compare_and_swap_default(remote_compare_and_swap_default_cb& cb) {
    return remote_compare_and_swap_default_handlers::start(cb);
}

mgbase::deferred<void> remote_fetch_and_add_default(remote_fetch_and_add_default_cb& cb) {
    return remote_fetch_and_add_default_handlers::start(cb);
}

mgbase::deferred<void> local_compare_and_swap_default(local_compare_and_swap_default_cb& cb) {
    return local_compare_and_swap_default_handlers::start(cb);
}

mgbase::deferred<void> local_fetch_and_add_default(local_fetch_and_add_default_cb& cb) {
    return local_fetch_and_add_default_handlers::start(cb);
}

} // namespace detail

} // namespace untyped
} // namespace rma
} // namespace mgcom


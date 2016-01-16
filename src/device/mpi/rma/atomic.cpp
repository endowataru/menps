
#include "atomic.ipp"

namespace mgcom {
namespace rma {

void initialize_atomic()
{
    emulated_atomic<atomic_default_t>::initialize();
}

namespace untyped {
namespace detail {

mgbase::deferred<void> remote_atomic_read_default(remote_atomic_read_default_cb& cb) {
    return emulated_atomic<atomic_default_t>::read(cb);
}
mgbase::deferred<void> remote_atomic_write_default(remote_atomic_write_default_cb& cb) {
    return emulated_atomic<atomic_default_t>::write(cb);
}

mgbase::deferred<void> local_compare_and_swap_default(local_compare_and_swap_default_cb& cb) {
    return emulated_atomic<atomic_default_t>::compare_and_swap(cb, mgcom::current_process_id());
}
mgbase::deferred<void> local_fetch_and_add_default(local_fetch_and_add_default_cb& cb) {
    return emulated_atomic<atomic_default_t>::fetch_and_add(cb, mgcom::current_process_id());
}

mgbase::deferred<void> remote_compare_and_swap_default(remote_compare_and_swap_default_cb& cb) {
    return emulated_atomic<atomic_default_t>::compare_and_swap(cb, cb.target_proc);
}
mgbase::deferred<void> remote_fetch_and_add_default(remote_fetch_and_add_default_cb& cb) {
    return emulated_atomic<atomic_default_t>::fetch_and_add(cb, cb.target_proc);
}

} // namespace detail
} // namespace untyped

} // namespace rma
} // namespace mgcom


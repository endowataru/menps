
// DEPRECATED

#include "try_contiguous.ipp"

namespace mgcom {
namespace rma {
namespace untyped {

namespace detail {

mgbase::deferred<void> remote_read(remote_read_cb& cb) {
    return remote_read_handlers::start(cb);
}

mgbase::deferred<void> remote_write(remote_write_cb& cb) {
    return remote_write_handlers::start(cb);
}

} // namespace detail

} // namespace untyped
} // namespace rma
} // namespace mgcom


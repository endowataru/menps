
#include "contiguous.ipp"
#include "rma.hpp"

namespace mgcom {
namespace rma {

namespace untyped {

namespace /*unnamed*/ {

emulated_contiguous g_emulated;

} // unnamed namespace

namespace detail {

mgbase::deferred<void> remote_read(remote_read_cb& cb)
{
    return emulated_contiguous::read_handlers<g_emulated>::start(cb);
}
mgbase::deferred<void> remote_write(remote_write_cb& cb)
{
    return emulated_contiguous::write_handlers<g_emulated>::start(cb);
}

} // namespace detail

} // namespace untyped

void initialize_contiguous()
{
    untyped::g_emulated.initialize();
}
void finalize_contiguous()
{
    // do nothing
}

} // namespace rma
} // namespace mgcom


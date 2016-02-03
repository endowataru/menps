
#include "collective.impl.hpp"

namespace mgcom {
namespace collective {

namespace untyped {

mgbase::deferred<void> broadcast(broadcast_cb& cb)
{
    return detail::broadcast_handlers::start(cb);
}

mgbase::deferred<void> allgather(allgather_cb& cb)
{
    return detail::allgather_handlers::start(cb);
}

} // namespace untyped

} // namespace collective
} // namespace mgcom


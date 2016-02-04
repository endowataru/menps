
#include "collective.impl.hpp"

namespace mgcom {
namespace collective {

mgbase::deferred<void> barrier_nb(barrier_cb& cb)
{
    return detail::barrier_handlers::start(cb);
}

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


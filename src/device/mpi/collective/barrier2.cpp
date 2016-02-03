
#include "barrier2.impl.hpp"

namespace mgcom {
namespace collective {

namespace detail {

namespace /*unnamed*/ {

barrier_impl g_impl;

} // unnamed namespace

} // namespace detail

mgbase::deferred<void> barrier_nb(barrier_cb& cb)
{
    return detail::barrier_impl::handlers<detail::g_impl>::start(cb);
}

void initialize()
{
    detail::g_impl.initialize();
}

void finalize()
{
    // do nothing
}

} // namespace collective
} // namespace mgcom


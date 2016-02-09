
#include "barrier2.impl.hpp"

namespace mgcom {
namespace collective {

namespace /*unnamed*/ {

barrier_impl g_impl;

} // unnamed namespace

void barrier()
{
    barrier_cb cb;
    barrier_impl::handlers<g_impl>::start(cb).wait();
}

void initialize()
{
    g_impl.initialize();
}

void finalize()
{
    // do nothing
}

} // namespace collective
} // namespace mgcom


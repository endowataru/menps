
#include "contiguous.impl.hpp"

namespace mgcom {
namespace mpi {
namespace rma {

namespace /*unnamed*/ {

emulated_contiguous g_impl;

} // unnamed namespace

void initialize_contiguous()
{
    emulated_contiguous::initialize<g_impl>();
}

void finalize_contiguous()
{
    g_impl.finalize();
}

bool try_read_async(const untyped::read_params& params)
{
    return emulated_contiguous::try_read<g_impl>(params);
}

bool try_write_async(const untyped::write_params& params)
{
    return emulated_contiguous::try_write<g_impl>(params);
}

} // namespace rma
} // namespace mpi
} // namespace mgcom


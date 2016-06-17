
#include "atomic.impl.hpp"

namespace mgcom {
namespace mpi {
namespace rma {

void initialize_atomic()
{
    emulated_atomic<atomic_default_t>::initialize();
}

void finalize_atomic()
{
    emulated_atomic<atomic_default_t>::finalize();
}

MGBASE_WARN_UNUSED_RESULT
bool try_atomic_read_async(const atomic_read_params<atomic_default_t>& params)
{
    return emulated_atomic<atomic_default_t>::try_read(params);
}

MGBASE_WARN_UNUSED_RESULT
bool try_atomic_write_async(const atomic_write_params<atomic_default_t>& params)
{
    return emulated_atomic<atomic_default_t>::try_write(params);
}

MGBASE_WARN_UNUSED_RESULT
bool try_compare_and_swap_async(const compare_and_swap_params<atomic_default_t>& params)
{
    return emulated_atomic<atomic_default_t>::try_compare_and_swap(params);
}

MGBASE_WARN_UNUSED_RESULT
bool try_fetch_and_add_async(const fetch_and_add_params<atomic_default_t>& params)
{
    return emulated_atomic<atomic_default_t>::try_fetch_and_add(params);
}

} // namespace rma
} // namespace mpi
} // namespace mgcom


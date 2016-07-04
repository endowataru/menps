
#pragma once

#include <mgcom/rma/try_rma.hpp>

namespace mgcom {

namespace ibv {

// /*???*/ g_proxy;

namespace rma {

namespace untyped {

MGBASE_WARN_UNUSED_RESULT
bool try_read_async(const untyped::read_params& params) {
    return mgcom::ibv::g_proxy->try_read_async(params);
}

MGBASE_WARN_UNUSED_RESULT
bool try_write_async(const untyped::write_params& params) {
    return mgcom::ibv::g_proxy->try_write_async(params);
}

} // namespace untyped

MGBASE_WARN_UNUSED_RESULT
bool try_atomic_read_async(const atomic_read_params<atomic_default_t>& params) {
    return mgcom::ibv::g_proxy->try_atomic_read_async(params);
}

MGBASE_WARN_UNUSED_RESULT
bool try_atomic_write_async(const atomic_write_params<atomic_default_t>& params) {
    return mgcom::ibv::g_proxy->try_atomic_write_async(params);
}

MGBASE_WARN_UNUSED_RESULT
bool try_compare_and_swap_async(const rma::compare_and_swap_params<atomic_default_t>& params) {
    return mgcom::ibv::g_proxy->try_compare_and_swap_async(params);
}

MGBASE_WARN_UNUSED_RESULT
bool try_fetch_and_add_async(const rma::fetch_and_add_params<atomic_default_t>& params) {
    return mgcom::ibv::g_proxy->try_fetch_and_add_async(params);
}

} // namespace rma
} // namespace ibv
} // namespace mgcom


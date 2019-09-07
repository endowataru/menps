
#pragma once

#include <menps/medsm2/common.hpp>
#include <cmpth/prof.hpp>

#define MEDSM2_PROF_KINDS(x) \
    x(omp_barrier) \
    x(barrier) \
    x(fence_release) \
    x(release) \
    x(release_fast) \
    x(release_tx) \
    x(lock_global) \
    x(begin_tx) \
    x(tx_merge) \
    x(end_tx) \
    x(unlock_global) \
    x(tx_migrate) \
    x(tx_merge_memcmp) \
    x(tx_merge_local_memcpy) \
    x(tx_merge_remote_memcpy) \
    x(write_merge_simd) \
    x(write_merge_byte) \
    x(read_merge) \
    x(barrier_allgather) \
    x(barrier_acq) \
    x(push_wn) \
    x(start_write_twin) \
    x(tx_read) \
    x(merge_read) \
    x(wn_read) \
    x(read_upgrade) \
    x(write_upgrade) \
    x(mprotect_invalidate) \
    x(mprotect_start_read) \
    x(mprotect_start_write) \
    x(mprotect_tx_before) \
    x(mprotect_tx_after)

namespace menps {
namespace medsm2 {

struct prof_aspect_policy
{
    CMPTH_DEFINE_PROF_ASPECT_POLICY(MEDSM2_PROF_KINDS)
    
    static constexpr fdn::size_t get_default_mlog_size() noexcept {
        return 1ull << 20;
    }
};

} // namespace medsm2
} // namespace menps


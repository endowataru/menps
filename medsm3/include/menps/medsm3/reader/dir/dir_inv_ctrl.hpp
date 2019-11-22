
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class dir_inv_ctrl
{
    CMPTH_DEFINE_DERIVED(P)

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using blk_global_lock_type = typename P::blk_global_lock_type;

    using sharer_map_type = typename P::sharer_map_type;

    using ult_itf_type = typename P::ult_itf_type;
    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

public:
    struct try_invalidate_result {
        bool    needs_invalidate;
    };

    try_invalidate_result try_invalidate(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();
        const auto invalidated = self.read_local_inv_flag(blk_llk);
        return { invalidated };
    }

    template <typename StateDataResult>
    sharer_map_type send_inv_to_sharers(
        blk_global_lock_type&   blk_glk
    ,   const StateDataResult&  sd_ret
    ) {
        auto& self = this->derived();
        auto& blk_llk = blk_glk.local_lock();
        auto& com = blk_llk.get_com_itf();
        const auto this_proc = com.this_proc_id();

        auto sharers = blk_glk.get_sharer_map();
        if (sd_ret.is_written) {
            auto& rma = com.get_rma();
            const auto num_procs = com.get_num_procs();

            std::vector<proc_id_type> inv_sharers;
            for (proc_id_type proc_id = 0; proc_id < num_procs; ++proc_id) {
                if (proc_id != this_proc && sharers.is_set(proc_id)) {
                    inv_sharers.push_back(proc_id);
                    sharers.unset(proc_id);
                }
            }

            // Send invalidation messages.
            ult_itf_type::for_loop(
                ult_itf_type::execution::par
            ,   0, inv_sharers.size()
            ,   [&] (const fdn::size_t i) {
                    const bool written_flag = true;
                    const auto proc_id = inv_sharers[i];
                    const auto flag_rptr = self.get_remote_inv_flag_ptr(proc_id, blk_llk);
                    CMPTH_P_LOG_DEBUG(P
                    ,   "Send invalidation."
                    ,   "blk_id", blk_llk.blk_id().to_str()
                    ,   "proc_id", proc_id
                    );
                    rma.buf_write(proc_id, flag_rptr, &written_flag, 1);
                }
            );
        }
        
        // Set this process as a sharer.
        sharers.set(this_proc);

        // Reset the invalidation flag for this process.
        self.reset_local_inv_flag(blk_llk);

        return sharers;
    }
};

} // namespace medsm3
} // namespace menps



#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class ts_home_ctrl
{
    CMPTH_DEFINE_DERIVED(P)

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using blk_global_lock_type = typename P::blk_global_lock_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

    using global_entry_type = typename P::global_entry_type;

public:
    blk_global_lock_type get_global_lock(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();
        auto lk_ret = this->lock_global(blk_llk);
        return blk_global_lock_type{
            blk_llk, lk_ret.owner_proc, self, lk_ret.ge
        };
    }

    struct lock_global_result {
        proc_id_type        owner_proc;
        global_entry_type   ge;
    };

    lock_global_result lock_global(blk_local_lock_type& blk_llk)
    {
        CMPTH_P_LOG_DEBUG(P
        ,   "Locking global block lock."
        ,   "blk_id", blk_llk.blk_id().to_str()
        );

        auto& self = this->derived();
        auto glk_ret = self.lock_global_raw(blk_llk);

        const auto owner_proc = glk_ret.owner_proc;
        // TODO: Fix RMA interface (of UCT) to simplify the below procedure.
        // First, implicitly cast to byte*.
        const fdn::byte* const lad_ptr = glk_ret.owner_lad.get();
        // Then, cast byte* to global_entry_type*.
        const auto ge = *reinterpret_cast<const global_entry_type*>(lad_ptr);
        
        return { owner_proc, ge };
    }

    void unlock_global(blk_local_lock_type& blk_llk, const global_entry_type& ge)
    {
        CMPTH_P_LOG_DEBUG(P
        ,   "Unlocking global block lock."
        ,   "blk_id", blk_llk.blk_id().to_str()
        ,   "last_writer_proc", ge.last_writer_proc
        ,   "owner_wr_ts", ge.owner_intvl.wr_ts
        ,   "owner_rd_ts", ge.owner_intvl.rd_ts
        );

        auto& self = this->derived();
        self.unlock_global_raw(blk_llk, &ge);
    }
    
    global_entry_type read_lock_entry(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();
        global_entry_type ge = global_entry_type();
        self.read_lock_entry_raw(blk_llk, &ge);
        CMPTH_P_ASSERT(P, ge.last_writer_proc == blk_llk.get_com_itf().this_proc_id());
        return ge;
    }
    void write_lock_entry(blk_local_lock_type& blk_llk, const global_entry_type& ge)
    {
        auto& self = this->derived();
        CMPTH_P_ASSERT(P, ge.last_writer_proc == blk_llk.get_com_itf().this_proc_id());
        self.write_lock_entry_raw(blk_llk, &ge);
    }
};

} // namespace medsm3
} // namespace menps


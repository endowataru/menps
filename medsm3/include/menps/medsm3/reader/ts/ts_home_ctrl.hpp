
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

    using ts_interval_type = typename P::ts_interval_type;

public:
    blk_global_lock_type get_global_lock(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();
        auto lk_ret = this->lock_global(blk_llk);
        return blk_global_lock_type{
            blk_llk, lk_ret.owner_proc, lk_ret.owner_intvl, self
        };
    }

    struct lock_global_result {
        proc_id_type        owner_proc;
        ts_interval_type    owner_intvl;
    };

    lock_global_result lock_global(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();

        auto glk_ret = self.lock_global_raw(blk_llk);

        const auto owner_proc = glk_ret.owner_proc;
        // TODO: Fix RMA interface (of UCT) to simplify the below procedure.
        // First, implicitly cast to byte*.
        const fdn::byte* const owner_intvl_bptr = glk_ret.owner_lad.get();
        // Then, cast byte* to ts_interval_type*.
        const auto owner_intvl =
            *reinterpret_cast<const ts_interval_type*>(owner_intvl_bptr);
        
        return { owner_proc, owner_intvl };
    }

    void unlock_global(blk_local_lock_type& blk_llk, const ts_interval_type& owner_intvl)
    {
        auto& self = this->derived();
        
        self.unlock_global_raw(blk_llk, &owner_intvl);
    }
    
    ts_interval_type read_lock_entry(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();
        ts_interval_type intvl = ts_interval_type();
        self.read_lock_entry_raw(blk_llk, &intvl);
        return intvl;
    }
    void write_lock_entry(blk_local_lock_type& blk_llk, const ts_interval_type& intvl)
    {
        auto& self = this->derived();
        self.write_lock_entry_raw(blk_llk, &intvl);
    }
};

} // namespace medsm3
} // namespace menps


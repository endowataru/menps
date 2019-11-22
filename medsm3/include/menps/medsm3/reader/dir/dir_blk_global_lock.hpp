
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class dir_blk_global_lock
    : public P::blk_global_lock_base_type
{
    using base = typename P::blk_global_lock_base_type;

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using home_ctrl_type = typename P::home_ctrl_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

    using global_entry_type = typename P::global_entry_type;
    using sharer_map_type = typename P::sharer_map_type;

public:
    dir_blk_global_lock() MEFDN_DEFAULT_NOEXCEPT = default;

    explicit dir_blk_global_lock(
        blk_local_lock_type&        blk_llk
    ,   const proc_id_type          owner
    ,   home_ctrl_type&             home_ctrl
    ,   global_entry_type&&         ge
    ) noexcept
        : base{blk_llk, owner, ge.last_writer_proc}
        , home_ctrl_{&home_ctrl}
        , sh_map_{fdn::move(ge.sh_map)}
    { }

    const sharer_map_type& get_sharer_map() const noexcept {
        return this->sh_map_;
    }
    
    void unlock(const global_entry_type& ge)
    {
        CMPTH_P_ASSERT(P, this->home_ctrl_ != nullptr);
        auto& blk_llk = this->local_lock();
        this->home_ctrl_->unlock_global(blk_llk, ge);
        this->release();
    }

private:
    home_ctrl_type* home_ctrl_ = nullptr;
    sharer_map_type sh_map_;
};

} // namespace medsm3
} // namespace menps


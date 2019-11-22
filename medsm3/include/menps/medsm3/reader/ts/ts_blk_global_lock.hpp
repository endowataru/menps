
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class ts_blk_global_lock
    : public P::blk_global_lock_base_type
{
    using base = typename P::blk_global_lock_base_type;

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using home_ctrl_type = typename P::home_ctrl_type;
    using ts_interval_type = typename P::ts_interval_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

    using global_entry_type = typename P::global_entry_type;

public:
    ts_blk_global_lock() MEFDN_DEFAULT_NOEXCEPT = default;

    explicit ts_blk_global_lock(
        blk_local_lock_type&        blk_llk
    ,   const proc_id_type          owner
    ,   home_ctrl_type&             home_ctrl
    ,   const global_entry_type&    home_ge
    ) noexcept
        : base{blk_llk, owner, home_ge.last_writer_proc}
        , home_ctrl_{&home_ctrl}
        , home_intvl_(home_ge.owner_intvl) // Note: {} cannot be used in GCC 4.8?
    { }

    ts_interval_type get_home_interval() const noexcept {
        CMPTH_P_ASSERT(P, this->owns_lock());
        return this->home_intvl_;
    }
    
    void unlock(const global_entry_type& ge)
    {
        CMPTH_P_ASSERT(P, this->home_ctrl_ != nullptr);
        auto& blk_llk = this->local_lock();
        this->home_ctrl_->unlock_global(blk_llk, ge);
        this->release();
    }
    
private:
    home_ctrl_type*     home_ctrl_ = nullptr;
    ts_interval_type    home_intvl_ = ts_interval_type();
};

} // namespace medsm3
} // namespace menps


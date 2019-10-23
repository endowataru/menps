
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

public:
    ts_blk_global_lock() MEFDN_DEFAULT_NOEXCEPT = default;

    explicit ts_blk_global_lock(
        blk_local_lock_type&    blk_llk
    ,   const proc_id_type      owner
    ,   const ts_interval_type  ts_intvl
    ,   home_ctrl_type&         home_ctrl
    ) noexcept
        : base{blk_llk, owner}
        , ts_intvl_{ts_intvl}
        , home_ctrl_{&home_ctrl}
    { }

    ts_interval_type get_home_interval() const noexcept {
        CMPTH_P_ASSERT(P, this->owns_lock());
        return this->ts_intvl_;
    }
    
    void unlock(const ts_interval_type new_intvl)
    {
        CMPTH_P_ASSERT(P, this->home_ctrl_ != nullptr);
        auto& blk_llk = this->local_lock();
        this->home_ctrl_->unlock_global(blk_llk, new_intvl);
        this->release();
    }
    
private:
    ts_interval_type    ts_intvl_ = ts_interval_type();
    home_ctrl_type*     home_ctrl_ = nullptr;
};


} // namespace medsm3
} // namespace menps


#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class blk_global_lock_base
{
    using blk_local_lock_type = typename P::blk_local_lock_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

protected:
    blk_global_lock_base() MEFDN_DEFAULT_NOEXCEPT = default;

    explicit blk_global_lock_base(
        blk_local_lock_type&    blk_llk
    ,   const proc_id_type      owner
    ,   const proc_id_type      last_writer_proc
    ) noexcept
        : blk_llk_{&blk_llk}
        , prev_owner_{owner}
        , last_writer_proc_{last_writer_proc}
    { }

    ~blk_global_lock_base() {
        CMPTH_P_ASSERT(P, !this->owns_lock());
    }

    void release() noexcept {
        CMPTH_P_ASSERT(P, this->owns_lock());
        this->blk_llk_ = nullptr;
    }

public:
    blk_local_lock_type& local_lock() noexcept {
        CMPTH_P_ASSERT(P, this->owns_lock());
        return *this->blk_llk_;
    }
    proc_id_type prev_owner() const noexcept {
        CMPTH_P_ASSERT(P, this->owns_lock());
        return this->prev_owner_;
    }
    proc_id_type last_writer_proc() const noexcept {
        CMPTH_P_ASSERT(P, this->owns_lock());
        return this->last_writer_proc_;
    }
    bool owns_lock() const noexcept {
        return this->blk_llk_ != nullptr;
    }
    
private:
    blk_local_lock_type*    blk_llk_ = nullptr;
    proc_id_type            prev_owner_ = proc_id_type();
    proc_id_type            last_writer_proc_ = proc_id_type();
};

} // namespace medsm3
} // namespace menps



#pragma once

#include <menps/medsm3/data/basic_data_ctrl.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class svm_data_ctrl
    : public basic_data_ctrl<P>
{
    using blk_local_lock_type = typename P::blk_local_lock_type;

    using segment_set_type = typename P::segment_set_type;
    using segment_set_ptr_type = typename P::segment_set_ptr_type;
    
    using com_itf_type = typename P::com_itf_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    template <typename T>
    using local_ptr = typename rma_itf_type::template local_ptr<T>;
    template <typename T>
    using remote_ptr = typename rma_itf_type::template remote_ptr<T>;

    using byte = fdn::byte;

    using constants_type = typename P::constants_type;

public:
    explicit svm_data_ctrl(segment_set_ptr_type seg_set_ptr)
        : seg_set_ptr_{fdn::move(seg_set_ptr)}
    { }

    bool is_migration_enabled() const noexcept {
        return constants_type::is_migration_enabled;
    }
    bool is_lazy_merge_enabled() const noexcept {
        return constants_type::is_lazy_merge_enabled;
    }

    local_ptr<byte> get_local_working_lptr(blk_local_lock_type& blk_llk) {
        auto& seg = this->seg_set().segment_of(blk_llk);
        return seg.get_local_working_lptr(blk_llk.subindex());
    }
    remote_ptr<byte> get_remote_source_rptr(const proc_id_type proc, blk_local_lock_type& blk_llk) {
        auto& seg = this->seg_set().segment_of(blk_llk);
        if (this->is_lazy_merge_enabled()) {
            return seg.get_remote_working_rptr(proc, blk_llk.subindex());
        }
        else {
            return seg.get_remote_primary_rptr(proc, blk_llk.subindex());
        }
    }
    byte* get_local_snapshot_ptr(blk_local_lock_type& blk_llk) {
        auto& seg = this->seg_set().segment_of(blk_llk);
        if (this->is_migration_enabled()) {
            return seg.get_local_primary_lptr(blk_llk.subindex());
        }
        else {
            return seg.get_local_secondary_ptr(blk_llk.subindex());
        }
    }
    remote_ptr<byte> get_remote_snapshot_rptr(const proc_id_type proc, blk_local_lock_type& blk_llk) {
        CMPTH_P_ASSERT(P, this->is_migration_enabled() && this->is_lazy_merge_enabled());
        auto& seg = this->seg_set().segment_of(blk_llk);
        return seg.get_remote_primary_rptr(proc, blk_llk.subindex());
    }
    
    void protect_invalid(blk_local_lock_type& blk_llk) {
        this->protect(blk_llk, PROT_NONE);
    }
    void protect_readonly(blk_local_lock_type& blk_llk) {
        this->protect(blk_llk, PROT_READ);
    }
    void protect_writable(blk_local_lock_type& blk_llk) {
        this->protect(blk_llk, PROT_READ | PROT_WRITE);
    }

private:
    void protect(blk_local_lock_type& blk_llk, const int prot) {
        auto& seg = this->seg_set().segment_of(blk_llk);
        const auto blk_size = seg.get_blk_size();
        const auto app_ptr = seg.get_local_working_app_ptr(blk_llk.subindex());
        this->call_mprotect(app_ptr, blk_size, prot);
    }

    void call_mprotect(void* const p, const fdn::size_t num_bytes, const int prot)
    {
        const auto ret = mprotect(p, num_bytes, prot);
        
        if (ret == 0) {
            CMPTH_P_LOG_INFO(P
            ,   "Called mprotect()."
            ,   "ptr", p
            ,   "num_bytes", num_bytes
            ,   "prot", prot
            );
        }
        else {
            CMPTH_P_LOG_FATAL(P
            ,   "mprotect() failed."
            ,   "ptr", p
            ,   "num_bytes", num_bytes
            ,   "prot", prot
            ,   "ret", ret
            ,   "errno", errno
            );
            
            throw std::exception{};
        }
    }

private:
    segment_set_type& seg_set() noexcept {
        CMPTH_P_ASSERT(P, this->seg_set_ptr_);
        return *this->seg_set_ptr_;
    }
    segment_set_ptr_type seg_set_ptr_;
};

} // namespace medsm3
} // namespace menps


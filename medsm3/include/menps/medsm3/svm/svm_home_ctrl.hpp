
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class svm_home_ctrl
    : public P::home_ctrl_base_type
        // ts_home_ctrl or dir_home_ctrl
{
    using base = typename P::home_ctrl_base_type;

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using segment_set_type = typename P::segment_set_type;
    using segment_set_ptr_type = typename P::segment_set_ptr_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;
    template <typename T>
    using unique_public_ptr = typename rma_itf_type::template unique_public_ptr<T>;

    using byte = fdn::byte;

public:
    explicit svm_home_ctrl(segment_set_ptr_type seg_set_ptr)
        : seg_set_ptr_{fdn::move(seg_set_ptr)}
    { }

    bool check_owned(blk_local_lock_type& blk_llk)
    {
        auto& seg = this->seg_set().segment_of(blk_llk);
        auto& glk_tbl = seg.get_global_lock_table();
        const auto blk_sidx = blk_llk.subindex();

        auto& com = blk_llk.get_com_itf();

        return glk_tbl.check_owned(com, blk_sidx);
    }

private:
    friend base;

    struct lock_global_raw_result
    {
        proc_id_type                owner_proc;
        unique_public_ptr<byte []>   owner_lad;
    };

    lock_global_raw_result lock_global_raw(blk_local_lock_type& blk_llk)
    {
        auto& seg = this->seg_set().segment_of(blk_llk);
        auto& glk_tbl = seg.get_global_lock_table();
        const auto blk_sidx = blk_llk.subindex();
        const auto lad_size = glk_tbl.get_lad_size();
        
        auto& com = blk_llk.get_com_itf();
        auto& rma = com.get_rma();
        auto& p2p = com.get_p2p();
        const auto tag = blk_llk.lock_tag();
        
        auto owner_lad =
            rma.template make_unique_uninitialized<byte []>(lad_size);
        
        const auto ret = glk_tbl.lock_global(com, p2p, blk_sidx, tag, owner_lad.get());

        return { ret.owner, fdn::move(owner_lad) };
    }
    void unlock_global_raw(blk_local_lock_type& blk_llk, const void* const lad)
    {
        auto& seg = this->seg_set().segment_of(blk_llk);
        auto& glk_tbl = seg.get_global_lock_table();
        const auto blk_sidx = blk_llk.subindex();

        auto& com = blk_llk.get_com_itf();
        auto& p2p = com.get_p2p();
        const auto tag = blk_llk.lock_tag();
        
        glk_tbl.unlock_global(com, p2p, blk_sidx, tag, lad);
    }

    void read_lock_entry_raw(blk_local_lock_type& blk_llk, void* const lad_dest)
    {
        auto& seg = this->seg_set().segment_of(blk_llk);
        auto& glk_tbl = seg.get_global_lock_table();
        const auto blk_sidx = blk_llk.subindex();
        const auto lad_size = glk_tbl.get_lad_size();

        const void* const lad_src = glk_tbl.get_local_lad_at(blk_sidx);
        // TODO: atomicity
        std::memcpy(lad_dest, lad_src, lad_size);
    }
    void write_lock_entry_raw(blk_local_lock_type& blk_llk, const void* const lad_src)
    {
        auto& seg = this->seg_set().segment_of(blk_llk);
        auto& glk_tbl = seg.get_global_lock_table();
        const auto blk_sidx = blk_llk.subindex();
        const auto lad_size = glk_tbl.get_lad_size();

        void* const lad_dest = glk_tbl.get_local_lad_at(blk_sidx);
        // TODO: atomicity
        std::memcpy(lad_dest, lad_src, lad_size);
    }

    segment_set_type& seg_set() const noexcept { return *this->seg_set_ptr_; }

    segment_set_ptr_type seg_set_ptr_;
};

} // namespace medsm3
} // namespace menps



#pragma once

#include <menps/medsm3/layout/basic_segment.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class dir_segment
    : public basic_segment<P>
{
    using base = basic_segment<P>;

    using com_itf_type = typename P::com_itf_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

    using sharer_map_type = typename P::sharer_map_type;

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using blk_id_type = typename P::blk_id_type;

    using wr_count_type = typename P::wr_count_type;
    template <typename T>
    using remote_ptr = typename rma_itf_type::template remote_ptr<T>;
    template <typename T>
    using alltoall_buffer_t = typename com_itf_type::template alltoall_buffer_t<T>;
    
public:
    explicit dir_segment(com_itf_type& com, const fdn::size_t num_blks)
        : base{com, num_blks, dir_segment::get_lad_size(com)}
        , lk_entries_{fdn::make_unique<local_entry_type []>(num_blks)}
    {
        for (fdn::size_t sidx = 0; sidx < num_blks; ++sidx) {
            this->lk_entries_[sidx].fast_rel_threshold = 1;
        }

        auto& rma = com.get_rma();
        auto& coll = com.get_coll();
        this->inv_flags_.coll_make(rma, coll, num_blks);
    }

private:
    static fdn::size_t get_lad_size(com_itf_type& com) noexcept {
        return sizeof(proc_id_type) + sharer_map_type::get_sharer_map_size(com.get_num_procs());
    }

public:
    struct global_entry_type
    {
        proc_id_type    last_writer_proc;
        sharer_map_type sh_map;
    };

    struct local_entry_type
        : base::blk_local_metadata_base
    {
        // The number of transactions without writing the data.
        wr_count_type   wr_count;

        // TODO: These members are not necessary.
        bool            is_written_last;
        wr_count_type   fast_rel_count;
        wr_count_type   fast_rel_threshold;
    };
 
    local_entry_type& get_local_entry(blk_local_lock_type& blk_llk) {
        return this->get_local_entry_of_blk_id(blk_llk.blk_id());
    }   
    local_entry_type& get_local_entry_of_blk_id(const blk_id_type blk_id) {
        CMPTH_P_ASSERT(P, blk_id.sidx < this->get_num_blks());
        return this->lk_entries_[blk_id.sidx];
    }   

    bool* get_local_inv_flag_ptr(blk_local_lock_type& blk_llk) {
        const auto blk_id = blk_llk.blk_id();
        return this->inv_flags_.local(blk_id.sidx);
    }
    remote_ptr<bool> get_remote_inv_flag_ptr(
        const proc_id_type      proc
    ,   blk_local_lock_type&    blk_llk
    ) {
        const auto blk_id = blk_llk.blk_id();
        return this->inv_flags_.remote(proc, blk_id.sidx);
    }

private:
    fdn::unique_ptr<local_entry_type []> lk_entries_;
    alltoall_buffer_t<bool> inv_flags_;
};

} // namespace medsm3
} // namespace menps

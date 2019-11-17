
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class basic_segment_set
{
    CMPTH_DEFINE_DERIVED(P)

    using blk_id_type = typename P::blk_id_type;
    using seg_id_type = typename P::seg_id_type;
    using blk_subindex_type = typename P::blk_subindex_type;
    using segment_type = typename P::segment_type;

    using blk_local_lock_type = typename P::blk_local_lock_type;

    using ult_itf_type = typename P::ult_itf_type;
    using mutex_type = typename ult_itf_type::mutex;
    using unique_lock_type = typename ult_itf_type::template unique_lock<mutex_type>;

public:
    explicit basic_segment_set(fdn::size_t max_num_segs)
        : max_num_segs_{max_num_segs}
        , segs_{fdn::make_unique<fdn::unique_ptr<segment_type> []>(max_num_segs)}
    {
        CMPTH_P_ASSERT(P, max_num_segs > 0);
    }

    void* coll_make_segment(const fdn::size_t seg_size, const fdn::size_t blk_size)
    {
        auto& self = this->derived();

        // Generate the next segment ID.
        const auto seg_id = this->new_seg_id_++;
        CMPTH_P_LOG_VERBOSE(P
        ,   "Make new segment."
        ,   "seg_id", seg_id
        ,   "seg_size", seg_size
        ,   "blk_size", blk_size
        );
        CMPTH_P_ASSERT_ALWAYS(P, this->is_valid_seg_id(seg_id));
        
        const auto seg_conf = self.make_segment_conf(seg_id, seg_size, blk_size);
        auto seg = fdn::make_unique<segment_type>(seg_conf);

        const auto ret = seg->get_local_working_app_ptr();
        this->segs_[seg_id-1] = fdn::move(seg);

        return ret;
    }
    void coll_make_global_var_segment(
        const fdn::size_t   seg_size
    ,   const fdn::size_t   blk_size
    ,   void* const         start_ptr
    ) {
        auto& self = this->derived();

        const seg_id_type seg_id = 1;
        CMPTH_P_LOG_VERBOSE(P,
            "Make global variable segment."
        ,   "seg_size", seg_size
        ,   "blk_size", blk_size
        ,   "start_ptr", start_ptr
        );
        CMPTH_P_ASSERT_ALWAYS(P, !this->segs_[seg_id]);
        
        const auto seg_conf = self.make_global_var_segment_conf(seg_size, blk_size, start_ptr);
        auto seg = fdn::make_unique<segment_type>(seg_conf);
        this->segs_[seg_id-1] = fdn::move(seg);
    }

    unique_lock_type get_flag_set_lock() {
        return unique_lock_type{this->flag_set_mtx_};
    }

    fdn::size_t get_max_num_segs() const noexcept {
        return this->max_num_segs_;
    }

    // Expose this type to other classes.
    using local_entry_type = typename segment_type::local_entry_type;

    template <typename T>
    T get_local_entry_of(blk_local_lock_type& blk_llk, T local_entry_type::* const mem) {
        auto& seg = this->segment_of(blk_llk);
        auto& blk_le = seg.get_local_entry(blk_llk);
        return blk_le.*mem;
    }
    template <typename T>
    void set_local_entry_of(blk_local_lock_type& blk_llk, T local_entry_type::* const mem, const T val) {
        auto& seg = this->segment_of(blk_llk);
        auto& blk_le = seg.get_local_entry(blk_llk);
        blk_le.*mem = val;
    }

    segment_type& segment_of(blk_local_lock_type& blk_llk) {
        return this->segment_of_blk_id(blk_llk.blk_id());
    }
    segment_type& segment_of_blk_id(const blk_id_type& blk_id) {
        return this->segment_of_seg_id(blk_id.seg_id);
    }

protected:
    bool try_get_blk_id_from(const seg_id_type seg_id, const void* const ptr, blk_id_type* const out_blk_id) {
        auto& seg = this->segment_of_seg_id(seg_id);
        blk_subindex_type sidx = 0;
        if (MEFDN_UNLIKELY(!seg.try_get_subindex_from_app_ptr(ptr, &sidx))) {
            return false;
        }
        *out_blk_id = { seg_id, sidx };
        CMPTH_P_ASSERT(P, P::is_valid_blk_id(*out_blk_id));
        return true;
    }
    bool is_valid_seg_id(const seg_id_type seg_id) const noexcept {
        return 0 < seg_id && seg_id <= this->get_max_num_segs();
    }

private:
    segment_type& segment_of_seg_id(const seg_id_type seg_id) {
        CMPTH_P_ASSERT(P, this->is_valid_seg_id(seg_id));
        const auto& seg_ptr = this->segs_[seg_id-1];
        CMPTH_P_ASSERT(P, seg_ptr);
        return *seg_ptr;
    }

    fdn::size_t max_num_segs_ = 0;
    fdn::unique_ptr<fdn::unique_ptr<segment_type> []> segs_;
    seg_id_type new_seg_id_ = 2;

    mutex_type flag_set_mtx_;
};

} // namespace medsm3
} // namespace menps


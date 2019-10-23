
#pragma once

#include <menps/medsm3/layout/basic_segment_set.hpp>

namespace menps {
namespace medsm3 {

struct invalid_address_error : std::exception {};

template <typename P>
class svm_segment_set
    : public basic_segment_set<P>
{
    using base = basic_segment_set<P>;

    using blk_id_type = typename P::blk_id_type;
    using seg_id_type = typename P::seg_id_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

    using shm_object_type = typename P::shm_object_type;

    using constants_type = typename P::constants_type;

public:
    explicit svm_segment_set(
        com_itf_type&       com
    ,   const fdn::size_t   max_num_segs
    )
        : base{max_num_segs}
        , com_(com)
    {
        const auto this_proc = this->com_.this_proc_id();
        const auto reg_name = this->get_reg_name(this_proc);
        this->shm_obj_ =
            mefdn::make_unique<shm_object_type>(
                typename shm_object_type::config{
                    reg_name.c_str()
                ,   this->get_space_size()
                }
            );
    }

    struct segment_conf {
        com_itf_type&   com;
        fdn::size_t     num_blks;
        fdn::size_t     blk_size;
        fdn::byte*      working_app_ptr;
        fdn::byte*      working_sys_ptr;
        fdn::byte*      primary_ptr;
        fdn::byte*      secondary_ptr;
        int             fd;
        bool            is_copied;
        bool            is_migration_enabled;
    };

    segment_conf make_segment_conf(
        const seg_id_type seg_id
    ,   const fdn::size_t seg_size
    ,   const fdn::size_t blk_size
    ) {
        const auto max_seg_size = this->get_max_seg_size();
        const auto max_num_segs = this->get_max_num_segs();

        const auto byte_null = static_cast<fdn::byte*>(nullptr);
        const auto working_app_ptr = &byte_null[ (seg_id-1)                   * max_seg_size];
        const auto working_sys_ptr = &byte_null[((seg_id-1) +   max_num_segs) * max_seg_size];
        const auto primary_ptr     = &byte_null[((seg_id-1) + 2*max_num_segs) * max_seg_size];
        const auto secondary_ptr   = &byte_null[((seg_id-1) + 3*max_num_segs) * max_seg_size];

        const auto num_blks = fdn::roundup_divide(seg_size, blk_size);
        
        return {
            this->get_com_itf()
        ,   num_blks
        ,   blk_size
        ,   working_app_ptr
        ,   working_sys_ptr
        ,   primary_ptr
        ,   secondary_ptr
        ,   this->shm_obj_->get_fd()
        ,   false // No need to copy normal segments on initialization
        ,   this->is_migration_enabled()
        };
    }

    segment_conf make_global_var_segment_conf(
        const fdn::size_t   seg_size
    ,   const fdn::size_t   blk_size
    ,   void* const         start_ptr
    ) {
        const auto conf = this->make_segment_conf(1, seg_size, blk_size);
        const auto offset = reinterpret_cast<fdn::ptrdiff_t>(start_ptr);
        return {
            conf.com
        ,   conf.num_blks
        ,   conf.blk_size
        ,   conf.working_app_ptr + offset
        ,   conf.working_sys_ptr + offset
        ,   conf.primary_ptr     + offset
        ,   conf.secondary_ptr   + offset
        ,   conf.fd
        ,   true // Need to copy the global segment
        ,   conf.is_migration_enabled
        };
    }

    bool try_get_blk_id_from_app_ptr(void* const ptr, blk_id_type* const out_blk_id)
    {
        const auto iptr = reinterpret_cast<fdn::uintptr_t>(ptr);
        const auto seg_id = static_cast<seg_id_type>(iptr >> this->get_ln2_max_seg_size()) + 1;
        if (MEFDN_UNLIKELY(!this->is_valid_seg_id(seg_id))) {
            return false;
        }
        return base::try_get_blk_id_from(seg_id, ptr, out_blk_id);
    }

    template <typename Func>
    void for_all_blocks_in_range(void* const ptr_void, const fdn::size_t size, Func func)
    {
        const auto first_ptr = reinterpret_cast<fdn::byte*>(ptr_void);
        const auto last_ptr = first_ptr + size - 1;
        // Exclude the byte at last because the range is [first_ptr, first_ptr+size).

        blk_id_type first_blk_id = blk_id_type();
        if (CMPTH_UNLIKELY(!this->try_get_blk_id_from_app_ptr(first_ptr, &first_blk_id))) {
            throw invalid_address_error{};
        }
        blk_id_type last_blk_id = blk_id_type();
        if (CMPTH_UNLIKELY(!this->try_get_blk_id_from_app_ptr(last_ptr, &last_blk_id))) {
            throw invalid_address_error{};
        }
        const auto seg_id = first_blk_id.seg_id;
        CMPTH_P_ASSERT_ALWAYS(P, seg_id == last_blk_id.seg_id);
        CMPTH_P_ASSERT(P, first_blk_id.sidx <= last_blk_id.sidx);

        for (auto sidx = first_blk_id.sidx; sidx <= last_blk_id.sidx; ++sidx) {
            func(blk_id_type{ seg_id, sidx });
        }
    }

    com_itf_type& get_com_itf() const noexcept { return this->com_; }
    bool is_migration_enabled() const noexcept {
        return constants_type::is_migration_enabled;
    }
    fdn::size_t get_max_seg_size() const noexcept {
        fdn::size_t one = 1;
        return one << this->get_ln2_max_seg_size();
    }
    fdn::size_t get_ln2_max_seg_size() const noexcept {
        return constants_type::ln2_max_seg_size;
    }

private:
    static std::string get_reg_name(const proc_id_type this_proc) {
        return fmt::format("medsm_cache_{}", this_proc);
    }
    fdn::size_t get_space_size() noexcept {
        return this->get_max_seg_size() * this->get_max_num_segs();
    }

    com_itf_type&                       com_;
    fdn::unique_ptr<shm_object_type>    shm_obj_;
};

} // namespace medsm3
} // namespace menps


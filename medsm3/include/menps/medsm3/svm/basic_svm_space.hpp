
#pragma once

#include <menps/medsm3/dsm_space.hpp>
#include <menps/medsm2/svm/sigsegv_catcher.hpp>
#include <menps/medsm2/svm/sigbus_catcher.hpp>

namespace menps {
namespace medsm3 {

struct unimplemented_error : std::exception { };

template <typename P>
class basic_svm_space
    : public dsm_space<P>
{
    using base = dsm_space<P>;

    using segment_set_type = typename P::segment_set_type;
    using segment_set_ptr_type = typename P::segment_set_ptr_type;

    using blk_id_type = typename P::blk_id_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using ult_itf_type = typename P::ult_itf_type;
    
    struct tss_policy {
        using value_type = basic_svm_space;
    };
    using tss_type =
        typename ult_itf_type::template thread_specific<tss_policy>;

    using sigsegv_catcher_type = medsm2::sigsegv_catcher;
    using sigbus_catcher_type = medsm2::sigbus_catcher;

    using pin_ctrl_ptr_type = typename P::pin_ctrl_ptr_type;
    using sync_table_ptr_type = typename P::sync_table_ptr_type;

    using prof_aspect_type = typename P::prof_aspect_type;

public:
    explicit basic_svm_space(
        segment_set_ptr_type    seg_set_ptr
    ,   pin_ctrl_ptr_type       pin_ctrl_ptr
    ,   sync_table_ptr_type     sync_tbl_ptr
    )
        : base{fdn::move(pin_ctrl_ptr), fdn::move(sync_tbl_ptr)}
        , seg_set_ptr_{fdn::move(seg_set_ptr)}
    {
        this->segv_catch_ =
            fdn::make_unique<sigsegv_catcher_type>(
                sigsegv_catcher_type::config{ on_fault{ *this }, false }
            );
        
        this->bus_catch_ =
            fdn::make_unique<sigbus_catcher_type>(
                sigbus_catcher_type::config{ on_fault{ *this }, false }
            );
    }
    
private:
    struct on_fault {
        basic_svm_space& self;
        bool operator() (void* const ptr, medsm2::fault_kind_t fault_kind) const {
            const auto tss_ptr = self.is_enabled_.get();
            if (MEFDN_LIKELY(tss_ptr != nullptr)) {
                MEFDN_ASSERT(tss_ptr == &self);
                if (!P::constants_type::enable_fault_classification) {
                    // Ignore fault kind.
                    fault_kind = medsm2::fault_kind_t::unknown;
                }
                if (fault_kind == medsm2::fault_kind_t::read) {
                    return self.try_upgrade_with_flag(ptr, false);
                }
                else if (fault_kind == medsm2::fault_kind_t::write) {
                    return self.try_upgrade_with_flag(ptr, true);
                }
                else {
                    return self.try_upgrade(ptr);
                }
            }
            else {
                return false;
            }
        }
    };

public:
    virtual ~basic_svm_space() /*noexcept*/ {
        auto& com = this->seg_set().get_com_itf();
        auto& coll = com.get_coll();
        const auto this_proc = com.this_proc_id();
        const auto num_procs = com.get_num_procs();
        for (proc_id_type proc = 0; proc < num_procs; ++proc) {
            if (proc == this_proc) {
                prof_aspect_type::print_all("medsm3", this_proc);
            }
            coll.barrier();
        }
    }

    virtual void* coll_alloc_seg(fdn::size_t seg_size, fdn::size_t blk_size) override
    {
        return this->seg_set().coll_make_segment(seg_size, blk_size);
    }

    virtual void coll_alloc_global_var_seg(
        const fdn::size_t   seg_size
    ,   const fdn::size_t   blk_size
    ,   void* const         start_ptr
    ) override
    {
        this->seg_set().coll_make_global_var_segment(seg_size, blk_size, start_ptr);
    }

    virtual bool compare_exchange_strong_acquire(
        mefdn::uint32_t&    /*target*/ // TODO: define atomic type
    ,   mefdn::uint32_t&    /*expected*/
    ,   mefdn::uint32_t     /*desired*/
    ) override {
        throw unimplemented_error{};
    }
    
    virtual void store_release(mefdn::uint32_t*, mefdn::uint32_t) override {
        throw unimplemented_error{};
    }
    
    virtual void pin(void* const ptr, const fdn::size_t size) override
    {
        this->seg_set().for_all_blocks_in_range(ptr, size,
            [this] (const blk_id_type blk_id) {
                this->start_pin_block(blk_id);
            });
    }
    
    virtual void unpin(void* const ptr, const fdn::size_t size) override
    {
        this->seg_set().for_all_blocks_in_range(ptr, size,
            [this] (const blk_id_type blk_id) {
                this->end_pin_block(blk_id);
            });
    }
    
    virtual void enable_on_this_thread() override {
        CMPTH_P_ASSERT(P, this->is_enabled_.get() == nullptr);
        this->is_enabled_.set(this);
    }
    virtual void disable_on_this_thread() override {
        CMPTH_P_ASSERT(P, this->is_enabled_.get() == this);
        this->is_enabled_.set(nullptr);
    }
    
    virtual bool try_upgrade(void* const ptr) override
    {
        blk_id_type blk_id = blk_id_type();
        if (CMPTH_UNLIKELY(!this->seg_set().try_get_blk_id_from_app_ptr(ptr, &blk_id))) {
            return false;
        }
        if (CMPTH_LIKELY(base::try_upgrade_block(blk_id))) {
            return true;
        }
        // Fatal error.
        abort();
    }

    com_itf_type& get_com_itf() { return this->seg_set().get_com_itf(); }

    virtual void enable_prof() noexcept override {
        prof_aspect_type::set_enabled(true);
    }
    virtual void disable_prof() noexcept override {
        prof_aspect_type::set_enabled(false);
    }

private:
    bool try_upgrade_with_flag(void* const ptr, const bool is_write)
    {
        blk_id_type blk_id = blk_id_type();
        if (CMPTH_UNLIKELY(!this->seg_set().try_get_blk_id_from_app_ptr(ptr, &blk_id))) {
            // ptr points to the outside of the DSM space.
            return false;
        }
        // This return value is discarded intentionally.
        const bool upgraded MEFDN_MAYBE_UNUSED =
            base::try_upgrade_block_with_flag(blk_id, is_write);
        // The returned boolean means that ptr points to the DSM space.
        return true;
    }

    segment_set_type& seg_set() noexcept {
        CMPTH_P_ASSERT(P, this->seg_set_ptr_);
        return *this->seg_set_ptr_;
    }

    segment_set_ptr_type    seg_set_ptr_;
    tss_type                is_enabled_;

    fdn::unique_ptr<sigsegv_catcher_type>   segv_catch_;
    fdn::unique_ptr<sigbus_catcher_type>    bus_catch_;
};

} // namespace medsm3
} // namespace menps



#pragma once

#include <menps/medsm2/dsm/space.hpp>
#include <menps/medsm2/svm/sigsegv_catcher.hpp>
#include <menps/medsm2/svm/sigbus_catcher.hpp>
#include <menps/medsm2/svm/shm_object.hpp>
#include <menps/mefdn/arithmetic.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class svm_space
    : public space<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using base = space<P>;
    
    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    
    using blk_tbl_type = typename P::blk_tbl_type;
    
    using size_type = typename P::size_type;
    using ptrdiff_type = typename P::ptrdiff_type;
    
    using seg_id_type = typename P::seg_id_type;
    using blk_id_type = typename P::blk_id_type;
    using sig_id_type = typename P::sig_id_type;
    
    using worker_ult_itf_type = typename P::worker_ult_itf_type;
    
    struct tss_policy {
        using value_type = derived_type;
    };
    
    using tss_type =
        typename worker_ult_itf_type::
            template thread_specific<tss_policy>;
    
public:
    template <typename Conf>
    explicit svm_space(const Conf& conf)
        : base(conf)
        , com_(conf.com)
    {
        const auto proc = this->com_.this_proc_id();
        
        shm_obj_ =
            mefdn::make_unique<shm_object>(
                shm_object::config{
                    this->get_reg_name(proc).c_str()
                ,   this->get_space_size()
                }
            );
        
        // Note: The derived class is not initialized yet.
        auto& self = this->derived();
        
        this->segv_catch_ =
            mefdn::make_unique<sigsegv_catcher>(
                sigsegv_catcher::config{ on_fault{ self }, false}
            );
        
        this->bus_catch_ =
            mefdn::make_unique<sigbus_catcher>(
                sigbus_catcher::config{ on_fault{ self }, false}
            );
    }
    
private:
    struct on_fault {
        derived_type& self;
        bool operator() (void* const ptr) const {
            const auto tss_ptr = self.is_enabled_.get();
            if (tss_ptr != nullptr) {
                MEFDN_ASSERT(tss_ptr == &self);
                
                const auto ret = self.try_upgrade(ptr);
                
                return ret;
            }
            else {
                return false;
            }
        }
    };
    
public:
    ~svm_space()
    {
        // Destroy wr_set.
        // TODO: Use a better naming convention for initialization/finalization.
        base::finalize();
    }
    
    void* coll_alloc_seg(
        const size_type seg_size
    ,   const size_type blk_size
    ) {
        // Generate the next segment ID.
        // TODO: Check the max number of segments.
        const auto seg_id = this->new_seg_id_++;
        
        const auto priv_app_ptr = this->get_priv_app_ptr_from_seg(seg_id);
        const auto priv_sys_ptr = this->get_priv_sys_ptr_from_seg(seg_id);
        const auto pub_ptr = this->get_pub_ptr_from_seg(seg_id);
        #ifndef MEDSM2_ENABLE_MIGRATION
        const auto snapshot_ptr = this->get_snapshot_ptr_from_seg(seg_id);
        #endif
        
        this->coll_alloc_seg(seg_size, blk_size, seg_id,
            priv_app_ptr, priv_sys_ptr, pub_ptr,
            #ifndef MEDSM2_ENABLE_MIGRATION
            snapshot_ptr,
            #endif
            false);
        
        return priv_app_ptr;
    }
    
    void coll_alloc_global_var_seg(
        const size_type seg_size
    ,   const size_type blk_size
    ,   void* const     start_ptr
    ) {
        const seg_id_type seg_id = 0;
        
        const auto priv_sys_ptr =
            static_cast<mefdn::byte*>(this->get_priv_sys_ptr_from_seg(0))
            + reinterpret_cast<ptrdiff_type>(start_ptr);
        
        const auto pub_ptr =
            static_cast<mefdn::byte*>(this->get_pub_ptr_from_seg(0))
            + reinterpret_cast<ptrdiff_type>(start_ptr);
        
        #ifndef MEDSM2_ENABLE_MIGRATION
        const auto snapshot_ptr =
            static_cast<mefdn::byte*>(this->get_snapshot_ptr_from_seg(0))
            + reinterpret_cast<ptrdiff_type>(start_ptr);
        #endif
        
        this->coll_alloc_seg(seg_size, blk_size, seg_id,
            start_ptr, priv_sys_ptr, pub_ptr, 
            #ifndef MEDSM2_ENABLE_MIGRATION
            snapshot_ptr,
            #endif
            true);
    }
    
private:
    void coll_alloc_seg(
        const size_type     seg_size
    ,   const size_type     blk_size
    ,   const seg_id_type   seg_id
    ,   void* const         priv_app_ptr
    ,   void* const         priv_sys_ptr
    ,   void* const         pub_app_ptr
        #ifndef MEDSM2_ENABLE_MIGRATION
    ,   void* const         snapshot_ptr
        #endif
    ,   const bool          is_copied
    ) {
        struct conf {
            com_itf_type&   com;
            size_type       blk_size;
            size_type       num_blks;
            void*           priv_app_ptr;
            void*           priv_sys_ptr;
            void*           pub_ptr;
            #ifndef MEDSM2_ENABLE_MIGRATION
            void*           snapshot_ptr;
            #endif
            int             fd;
            bool            is_copied;
        };
        
        const auto num_blks = mefdn::roundup_divide(seg_size, blk_size);
        
        auto blk_tbl_ptr = mefdn::make_unique<blk_tbl_type>();
        
        blk_tbl_ptr->coll_make(
            conf{ this->com_, blk_size, num_blks,
                priv_app_ptr, priv_sys_ptr, pub_app_ptr,
                #ifndef MEDSM2_ENABLE_MIGRATION
                snapshot_ptr,
                #endif
                this->shm_obj_->get_fd(), is_copied }
        );
        
        base::set_blk_table(seg_id, mefdn::move(blk_tbl_ptr));
    }
    
    
public:
    com_itf_type& get_com_itf() {
        return this->com_;
    }
    
    template <typename T>
    bool compare_exchange_strong_acquire(
        T&      target // TODO: define atomic type
    ,   T&      expected
    ,   const T desired
    ) {
        const auto blk_id = this->get_blk_id_from_app_ptr(&target);
        const auto sig_id = this->get_sig_id(blk_id);
        const auto offset =
            static_cast<size_type>(
                reinterpret_cast<mefdn::byte*>(&target)
                - reinterpret_cast<mefdn::byte*>(blk_id) // TODO
            );
        
        return base::compare_exchange_strong_acquire(
            sig_id, blk_id, offset, expected, desired);
    }
    
    template <typename T>
    void store_release(
        T&      target // TODO: define atomic type
    ,   const T value
    ) {
        const auto blk_id = this->get_blk_id_from_app_ptr(&target);
        const auto sig_id = this->get_sig_id(blk_id);
        const auto offset =
            static_cast<size_type>(
                reinterpret_cast<mefdn::byte*>(&target)
                - reinterpret_cast<mefdn::byte*>(blk_id) // TODO
            );
        
        base::store_release(
            sig_id, blk_id, offset, value);
    }
    
private:
    
public:
    #if 0
    syn_id_type get_syn_id(const blk_id_type blk_id)
    {
        auto& com = this->get_com_itf();
        const auto num_procs = com.get_num_procs();
        
        const auto syn_val = blk_id % (this->get_max_num_syns_per_proc() * num_procs);
        
        const auto syn_proc = syn_val % num_procs;
        const auto syn_pos = syn_val / num_procs;
        
        return { syn_proc, syn_pos };
    }
    #endif
    
    #if 0
    template <typename T>
    void store_release(T* const ptr, const T val)
    {
        const auto blk_id = this->get_blk_id_from_app_ptr(ptr);
        
        base::store_release(blk_id,
            [=] { *ptr = val; });
    }
    
    template <typename T>
    T load_acquire(T* const ptr)
    {
        // Load the variable first.
        const auto ret = *ptr;
        
        const auto blk_id = this->get_blk_id_from_app_ptr(ptr);
        
        base::load_acquire(blk_id);
        
        return ret;
    }
    #endif
    
    void pin(void* const ptr, const size_type size)
    {
        this->for_all_blocks(ptr, size,
            [&] (const blk_id_type blk_id) {
                base::pin(blk_id);
            });
    }
    
    void unpin(void* const ptr, const size_type size)
    {
        this->for_all_blocks(ptr, size,
            [&] (const blk_id_type blk_id) {
                base::unpin(blk_id);
            });
    }
    
    void enable_on_this_thread() {
        this->is_enabled_.set(&derived());
    }
    void disable_on_this_thread() {
        this->is_enabled_.set(nullptr);
    }
    
    bool try_upgrade(void* const ptr) // XXX
    {
        const auto blk_id = this->get_blk_id_from_app_ptr(ptr);
        
        const auto rd_ret = base::start_read(blk_id);
        if (rd_ret.is_newly_read) {
            // This block was exactly read at this time.
            return true;
        }
        
        const auto wr_ret = base::start_write(blk_id);
        if (wr_ret.write.is_writable) {
            // This block was or is now writable.
            // This behavior allows the situation
            // where the block has already been writable.
            // because other threads may have changed the state to writable.
            return true;
        }
        
        // Fatal error.
        abort();
    }
    
private:
    static std::string get_reg_name(const proc_id_type proc) {
        return fmt::format("medsm_cache_{}", proc);
    }
    size_type get_max_seg_size() {
        return P::constants_type::max_seg_size;
    }
    size_type get_space_size() {
        return P::constants_type::max_space_size;
    }
    size_type get_max_num_segs() {
        return get_space_size() / get_max_seg_size();
    }
    
    blk_id_type get_blk_id_from_app_ptr(void* const ptr_void) {
        const auto ptr = reinterpret_cast<mefdn::byte*>(ptr_void);
        const auto diff = ptr - this->app_ptr_start_;
        
        // Calculate the segment ID from the address offset.
        // TODO: Remove this modulo operation.
        const auto seg_id = diff / this->get_max_seg_size();
        
        // Get the block size of this segment.
        // This requires a table lookup.
        // Currently there is no better idea to skip it.
        const auto blk_size = this->get_blk_size(seg_id);
        
        // Remove the offset.
        // TODO: Remove this modulo operation.
        //       If the block size is always power of 2, this can be converted to a couple of bit operations.
        const auto new_diff = diff - (diff % blk_size);
        
        return new_diff;
    }
    
    sig_id_type get_sig_id(const blk_id_type blk_id)
    {
        // Calculate the segment ID from the address offset.
        // TODO: Remove this modulo operation.
        const auto seg_id = blk_id / this->get_max_seg_size();
        
        const auto blk_pos = this->get_blk_pos(seg_id, blk_id);
        
        return blk_pos;
    }
    
    template <typename Func>
    void for_all_blocks(void* const ptr_void, const size_type size, Func func) {
        const auto ptr = reinterpret_cast<mefdn::byte*>(ptr_void);
        const auto diff = ptr - this->app_ptr_start_;
        const auto ssize = static_cast<decltype(diff)>(size);
        
        // Calculate the segment ID from the address offset.
        // TODO: Remove this modulo operation.
        const auto seg_id = diff / this->get_max_seg_size();
        
        // Get the block size of this segment.
        // This requires a table lookup.
        // Currently there is no better idea to skip it.
        const auto blk_size = this->get_blk_size(seg_id);
        
        for (auto d = diff; d < diff + ssize; d += blk_size)
        {
            const auto new_diff = d - (d % blk_size);
            
            func(new_diff);
        }
    }
    
    void* get_priv_app_ptr_from_seg(const seg_id_type seg_id) {
        return seg_id * this->get_max_seg_size() + app_ptr_start_;
    }
    void* get_priv_sys_ptr_from_seg(const seg_id_type seg_id) {
        return (seg_id + get_max_num_segs()) * this->get_max_seg_size() + app_ptr_start_;
    }
    void* get_pub_ptr_from_seg(const seg_id_type seg_id) {
        return (seg_id + 2*get_max_num_segs()) * this->get_max_seg_size() + app_ptr_start_;
    }
    #ifndef MEDSM2_ENABLE_MIGRATION
    void* get_snapshot_ptr_from_seg(const seg_id_type seg_id) {
        return (seg_id + 3*get_max_num_segs()) * this->get_max_seg_size() + app_ptr_start_;
    }
    #endif
    
    size_type get_max_num_syns_per_proc() {
        return P::constants_type::max_num_syns_per_proc;
    }
    
    com_itf_type& com_;
    
    seg_id_type new_seg_id_ = 1;
    mefdn::byte* app_ptr_start_ = nullptr;
    
    mefdn::unique_ptr<sigsegv_catcher>  segv_catch_;
    mefdn::unique_ptr<sigbus_catcher>   bus_catch_;
    mefdn::unique_ptr<shm_object>       shm_obj_;
    
    tss_type is_enabled_;
};

} // namespace medsm2
} // namespace menps



#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/external/malloc.h>
#include <menps/medsm2/prof.hpp>
#include <menps/medsm2/com/dsm_com_itf.hpp> // TODO: required for medev2::prof
#include <menps/mefdn/arithmetic.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class basic_dsm_facade
{
    using com_itf_type = typename P::com_itf_type;
    
    using ult_itf_type = typename com_itf_type::ult_itf_type;
    using thread_type = typename ult_itf_type::thread;
    using barrier_type = typename ult_itf_type::barrier;
    
    using svm_space_type = typename P::svm_space_type;
    using mutex_id_type = typename svm_space_type::mutex_id_t;
    
    using thread_num_type = typename P::thread_num_type;
    
public:
    explicit basic_dsm_facade(int* const argc, char*** const argv)
        : cc_{mefdn::make_unique<com_itf_type>(argc, argv)}
    {
        auto& com = *this->cc_;
        auto& coll = com.get_coll();
        const auto proc_id = coll.this_proc_id();
        
        this->sp_ = mefdn::make_unique<svm_space_type>(com);
        
        const auto n_ths_per_proc = P::get_num_threads_per_proc();
        this->bar_ = mefdn::make_unique<barrier_type>(n_ths_per_proc);
        
        this->master_ti_.th_num = proc_id;
        this->master_ti_.th_idx_in_proc = 0;
        this->cur_ti_.set(&this->master_ti_);
    }
    
    ~basic_dsm_facade()
    {
        auto& com = *this->cc_;
        auto& coll = com.get_coll();
        
        // Do a barrier before exiting.
        coll.barrier();
        
        if (coll.this_proc_id() == 0) {
            this->enable_on_this_thread();
            
            #ifndef MEDSM2_ENABLE_LINEAR_ALLOCATOR
            if (this->heap_ptr_ != nullptr) {
                destroy_mspace(this->heap_ms_);
                this->heap_ptr_ = nullptr;
            }
            #endif
            
            this->disable_on_this_thread();
            
            this->sp_->deallocate_mutex(this->heap_mtx_id_);
        }
        
        // Do a barrier before destroying the resources.
        coll.barrier();
    }
    
    void init_global_var_seg(void* const data_begin, void* const data_end, const mefdn::size_t blk_size)
    {
        const auto global_var_blk_size = MEDSM2_GLOBAL_VAR_BLOCK_SIZE;
        MEFDN_ASSERT(blk_size == global_var_blk_size); // TODO
        
        const auto p_begin = reinterpret_cast<mefdn::byte*>(data_begin);
        const auto p_end   = reinterpret_cast<mefdn::byte*>(data_end);
        
        MEFDN_ASSERT(reinterpret_cast<mefdn::uintptr_t>(data_begin) % global_var_blk_size == 0);
        
        const auto data_size = p_end - p_begin;
        
        if (data_size > 0) {
            MEFDN_LOG_VERBOSE(
                "msg:Initialize global variables.\t"
                "data_begin:0x{:x}\t"
                "data_end:0x{:x}\t"
                "data_size:0x{:x}\t"
            ,   reinterpret_cast<mefdn::uintptr_t>(data_begin)
            ,   reinterpret_cast<mefdn::uintptr_t>(data_end)
            ,   data_size
            );
            
            this->sp_->coll_alloc_global_var_seg(data_size, global_var_blk_size, data_begin);
        }
    }
    
    void* init_heap_seg(const mefdn::size_t seg_size, const mefdn::size_t blk_size)
    {
        auto& com = *this->cc_;
        auto& coll = com.get_coll();
        
        this->heap_size_ = seg_size;
        this->heap_ptr_ = this->sp_->coll_alloc_seg(seg_size, blk_size);
        
        #ifdef MEDSM2_ENABLE_LINEAR_ALLOCATOR
        // Use the first 8 bytes for holding the current consumed size.
        this->heap_used_ = static_cast<mefdn::size_t*>(this->heap_ptr_);
        this->heap_ptr_ = this->heap_used_+1;
        #endif
        
        if (coll.this_proc_id() == 0) {
            this->heap_mtx_id_ = this->sp_->allocate_mutex();
        }
        
        coll.broadcast(0, &this->heap_mtx_id_, 1);
        
        if (coll.this_proc_id() == 0) {
            this->enable_on_this_thread();
            
            *this->heap_used_ = 0;
            
            #ifndef MEDSM2_ENABLE_LINEAR_ALLOCATOR
            this->heap_ms_ = create_mspace_with_base(this->heap_ptr_, this->heap_size_, 1);
            #endif
            
            this->disable_on_this_thread();
        }
        
        return this->heap_ptr_;
    }
    
    void enable_on_this_thread() {
        this->sp_->enable_on_this_thread();
    }
    void disable_on_this_thread() {
        this->sp_->disable_on_this_thread();
    }
    
    void print_prof()
    {
        #if (defined(MEDSM2_ENABLE_PROF) || defined(MEDEV2_ENABLE_PROF) || defined(MEULT_ENABLE_PROF))
        auto& coll = this->get_com_itf().get_coll();
        using coll_t = typename com_itf_type::coll_itf_type;
        using proc_id_t = typename coll_t::proc_id_type;
        const auto num_procs = coll.get_num_procs();
        std::cout << std::flush;
        coll.barrier();
        for (proc_id_t proc = 0; proc < num_procs; ++proc) {
            if (coll.this_proc_id() == proc) {
                std::cout << fmt::format("- proc: {}\n", proc);
                #ifdef MEDSM2_ENABLE_PROF
                std::cout << medsm2::prof::to_string("    - ");
                #endif
                #ifdef MEDEV2_ENABLE_PROF
                std::cout << medev2::mpi::prof::to_string("    - ");
                #endif
                #ifdef MEULT_ENABLE_PROF
                std::cout << meult::prof::to_string("    - ");
                #endif
                std::cout << std::flush;
            }
            coll.barrier();
        }
        #endif
    }
    
    com_itf_type& get_com_itf() const noexcept {
        return *this->cc_;
    }
    svm_space_type& get_space() const noexcept {
        return *this->sp_;
    }
    
    void* coallocate(const mefdn::size_t num_bytes)
    {
        auto& com = *this->cc_;
        auto& coll = com.get_coll();
        
        void* ret = nullptr;
        if (coll.this_proc_id() == 0) {
            ret = this->allocate(num_bytes);
        }
        coll.broadcast(0, &ret, 1);
        
        return ret;
    }
    void codeallocate(void* const ptr)
    {
        auto& com = *this->cc_;
        auto& coll = com.get_coll();
        
        coll.barrier();
        if (coll.this_proc_id() == 0) {
            this->deallocate(ptr);
        }
        coll.barrier();
    }
    
    void* allocate(const mefdn::size_t num_bytes)
    {
        this->sp_->lock_mutex(this->heap_mtx_id_);
        #ifdef MEDSM2_ENABLE_LINEAR_ALLOCATOR
        const auto ptr = static_cast<mefdn::byte*>(this->heap_ptr_) + *this->heap_used_;
        // Fix alignment.
        *this->heap_used_ += mefdn::roundup_divide(num_bytes, 16ul) * 16ul; // TODO: avoid magic number
        if (*this->heap_used_ >= this->heap_size_) {
            throw std::bad_alloc();
        }
        #else
        void* ptr = mspace_malloc(this->heap_ms_, num_bytes);
        #endif
        this->sp_->unlock_mutex(this->heap_mtx_id_);
        return ptr;
    }
    void deallocate(void* const ptr) {
        #ifdef MEDSM2_ENABLE_LINEAR_ALLOCATOR
        // TODO: Reuse the buffer.
        (void)ptr;
        #else
        this->sp_->lock_mutex(this->heap_mtx_id_);
        mspace_free(this->heap_ms_, ptr);
        this->sp_->unlock_mutex(this->heap_mtx_id_);
        #endif
    }
    
    template <typename Func>
    void spmd_parallel(Func func)
    {
        this->barrier();
        
        this->is_parallel_ = true;
        
        auto& coll = this->get_com_itf().get_coll();
        const auto proc_id = coll.this_proc_id();
        
        const auto n_ths_per_proc = P::get_num_threads_per_proc();
        const auto tis = mefdn::make_unique<thread_info []>(n_ths_per_proc);
        
        for (thread_num_type i = 1; i < n_ths_per_proc; ++i) {
            auto& ti = tis[i];
            ti.th_num = proc_id * n_ths_per_proc + i;
            ti.th_idx_in_proc = i;
            ti.th = thread_type{
                [this, &ti, &func] {
                    this->cur_ti_.set(&ti);
                    this->enable_on_this_thread();
                    func();
                    this->disable_on_this_thread();
                }
            };
        }
        
        {
            auto& ti = tis[0];
            ti.th_num = proc_id * n_ths_per_proc + 0;
            ti.th_idx_in_proc = 0;
            this->cur_ti_.set(&ti);
            
            func();
            
            this->cur_ti_.set(&this->master_ti_);
        }
        
        for (thread_num_type i = 1; i < n_ths_per_proc; ++i) {
            auto& ti = tis[i];
            ti.th.join();
        }
        
        this->is_parallel_ = false;
        
        this->barrier();
    }
    
    void barrier()
    {
        if (this->is_parallel_) {
            this->bar_->arrive_and_wait();
            
            const auto& ti = this->get_thread_info();
            if (ti.th_num == 0) {
                this->sp_->barrier();
            }
            
            this->bar_->arrive_and_wait();
        }
        else {
            this->sp_->barrier();
        }
    }
    
    void local_barrier()
    {
        this->bar_->arrive_and_wait();
    }
    
    thread_num_type get_thread_num() {
        const auto& ti = this->get_thread_info();
        return ti.th_num;
    }
    thread_num_type max_num_threads() {
        auto& com = *this->cc_;
        auto& coll = com.get_coll();
        const auto num_procs = coll.get_num_procs();
        const auto n_ths_per_proc = P::get_num_threads_per_proc();
        return num_procs * n_ths_per_proc;
    }
    
private:
    struct thread_info {
        thread_num_type th_num;
        thread_num_type th_idx_in_proc;
        thread_type th;
    };
    
    thread_info& get_thread_info() {
        const auto ti_ptr = this->cur_ti_.get();
        MEFDN_ASSERT(ti_ptr != nullptr);
        return *ti_ptr;
    }
    
    mefdn::unique_ptr<com_itf_type>     cc_;
    mefdn::unique_ptr<svm_space_type>   sp_;
    
    mefdn::size_t   heap_size_ = 0;
    void*           heap_ptr_ = nullptr;
    #ifdef MEDSM2_ENABLE_LINEAR_ALLOCATOR
    mefdn::size_t*  heap_used_ = nullptr;
    #else
    mspace          heap_ms_;
    #endif
    
    mutex_id_type   heap_mtx_id_ = mutex_id_type();
    
    
    bool is_parallel_ = false;
    
    struct tss_policy {
        using value_type = thread_info;
    };
    
    using tss_type =
        typename ult_itf_type::template thread_specific<tss_policy>;
    
    thread_info master_ti_ = thread_info();
    tss_type    cur_ti_;
    
    mefdn::unique_ptr<barrier_type> bar_;
};

} // namespace medsm2
} // namespace menps


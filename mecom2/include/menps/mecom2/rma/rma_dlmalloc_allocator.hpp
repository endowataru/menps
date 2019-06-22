
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/external/malloc.h>
#include <menps/mefdn/mutex.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class rma_dlmalloc_allocator
{
    MEFDN_DEFINE_DERIVED(P)
    
    template <typename T>
    using public_ptr_t = typename P::template public_ptr<T>;
    
    using ult_itf_type = typename P::ult_itf_type;
    //using spinlock_type = typename ult_itf_type::spinlock;
    using spinlock_type = typename ult_itf_type::mutex;
    
    using size_type = typename P::size_type;
    
protected:
    void init_public_allocator(const size_type num_bytes)
    {
        auto& self = this->derived();
        
        this->size_ = num_bytes;
        
        #ifdef MECOM2_USE_WORKER_LOCAL_ALLOCATOR
        const auto num_workers = ult_itf_type::get_num_workers();
        this->wis_ = mefdn::make_unique<worker_info []>(num_workers);
        
        for (size_type i = 0; i < num_workers; ++i) {
            auto& wi = this->wis_[i];
            
            // Allocate a huge buffer.
            wi.buf = mefdn::make_unique_uninitialized<mefdn::byte []>(num_bytes);
            const auto buf_ptr = wi.buf.get();
            
            // Register to the RMA system.
            wi.ptr = self.attach(buf_ptr, buf_ptr+num_bytes);
            // Prepare mspace.
            wi.ms = create_mspace_with_base(buf_ptr, num_bytes, 1);
            
            wi.ptr.get_minfo()->alloc_id = i; // TODO
        }
        
        #else
        // Allocate a huge buffer.
        this->buf_ = mefdn::make_unique_uninitialized<mefdn::byte []>(num_bytes);
        const auto buf_ptr = this->buf_.get();
        
        // Register to the RMA system.
        this->ptr_ = self.attach(buf_ptr, buf_ptr+num_bytes);
        // Prepare mspace.
        this->ms_ = create_mspace_with_base(buf_ptr, num_bytes, 1);
        #endif
    }
    void deinit_public_allocator()
    {
        auto& self = this->derived();
        
        #ifdef MECOM2_USE_WORKER_LOCAL_ALLOCATOR
        const auto num_workers = ult_itf_type::get_num_workers();
        for (size_type i = 0; i < num_workers; ++i) {
            auto& wi = this->wis_[i];
            
            destroy_mspace(wi.ms);
            wi.ms = mspace();
            
            self.detach(wi.ptr);
            
            wi.buf.reset();
        }
        
        #else
        destroy_mspace(this->ms_);
        this->ms_ = mspace();
        
        self.detach(this->ptr_);
        
        this->buf_.reset();
        #endif
    }
    
public:
    public_ptr_t<void> untyped_allocate(const size_type size_in_bytes)
    {
        #ifdef MECOM2_USE_WORKER_LOCAL_ALLOCATOR
        auto& wi = this->wis_[ult_itf_type::get_worker_num()];
        
        mefdn::unique_lock<spinlock_type> lk(wi.lock);
        
        const auto ptr = static_cast<mefdn::byte*>(
            mspace_malloc(wi.ms, size_in_bytes)
        );
        if (ptr == nullptr) {
            throw std::bad_alloc();
        }
        
        const auto buf_ptr = wi.buf.get();
        if (!(buf_ptr <= ptr && ptr < buf_ptr + size_)) {
            throw std::bad_alloc();
        }
        
        const auto offset = ptr - buf_ptr;
        return wi.ptr + offset;
        
        #else
        mefdn::unique_lock<spinlock_type> lk(this->lock_);
        
        const auto ptr = static_cast<mefdn::byte*>(
            mspace_malloc(this->ms_, size_in_bytes)
        );
        if (ptr == nullptr) {
            throw std::bad_alloc();
        }
        
        const auto buf_ptr = buf_.get();
        if (!(buf_ptr <= ptr && ptr < buf_ptr + size_)) {
            throw std::bad_alloc();
        }
        
        const auto offset = ptr - buf_ptr;
        return this->ptr_ + offset;
        #endif
    }
    void untyped_deallocate(const public_ptr_t<void>& lptr)
    {
        #ifdef MECOM2_USE_WORKER_LOCAL_ALLOCATOR
        const auto alloc_id = lptr.get_minfo()->alloc_id;
        auto& wi = this->wis_[alloc_id];
        
        mefdn::unique_lock<spinlock_type> lk(wi.lock);
        
        const auto ptr = lptr.get();
        mspace_free(wi.ms, ptr);
        
        #else
        mefdn::unique_lock<spinlock_type> lk(this->lock_);
        
        const auto ptr = lptr.get();
        mspace_free(this->ms_, ptr);
        #endif
    }
    
private:
    #ifdef MECOM2_USE_WORKER_LOCAL_ALLOCATOR
    struct worker_info {
        spinlock_type                       lock;
        mefdn::unique_ptr<mefdn::byte []>   buf;
        public_ptr_t<mefdn::byte>           ptr;
        mspace                              ms;
    };
    
    mefdn::unique_ptr<worker_info []>   wis_;
    
    #else
    spinlock_type                       lock_;
    mefdn::unique_ptr<mefdn::byte []>   buf_;
    public_ptr_t<mefdn::byte>           ptr_;
    mspace                              ms_;
    #endif
    size_type                           size_;
};

} // namespace mecom2
} // namespace menps


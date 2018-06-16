
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/external/malloc.h>

namespace menps {
namespace mecom2 {

template <typename P>
class rma_dlmalloc_allocator
{
    MEFDN_DEFINE_DERIVED(P)
    
    template <typename T>
    using public_ptr_t = typename P::template public_ptr<T>;
    
    using ult_itf_type = typename P::ult_itf_type;
    using spinlock_type = typename ult_itf_type::spinlock;
    
    using size_type = typename P::size_type;
    
protected:
    void init_public_allocator(const size_type num_bytes)
    {
        auto& self = this->derived();
        
        this->size_ = num_bytes;
        
        // Allocate a huge buffer.
        this->buf_ = mefdn::make_unique_uninitialized<mefdn::byte []>(num_bytes);
        const auto buf_ptr = this->buf_.get();
        
        // Register to the RMA system.
        this->ptr_ = self.attach(buf_ptr, buf_ptr+num_bytes);
        // Prepare mspace.
        this->ms_ = create_mspace_with_base(buf_ptr, num_bytes, 1);
    }
    void deinit_public_allocator()
    {
        auto& self = this->derived();
        
        destroy_mspace(this->ms_);
        this->ms_ = mspace();
        
        self.detach(this->ptr_);
        
        this->buf_.reset();
    }
    
public:
    public_ptr_t<void> untyped_allocate(const size_type size_in_bytes)
    {
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
    }
    void untyped_deallocate(const public_ptr_t<void>& lptr)
    {
        mefdn::unique_lock<spinlock_type> lk(this->lock_);
        
        const auto ptr = lptr.get();
        mspace_free(this->ms_, ptr);
    }
    
private:
    spinlock_type                       lock_;
    mefdn::unique_ptr<mefdn::byte []>   buf_;
    public_ptr_t<mefdn::byte>           ptr_;
    mspace                              ms_;
    size_type                           size_;
};

} // namespace mecom2
} // namespace menps


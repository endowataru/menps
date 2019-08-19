
#pragma once

#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/logger.hpp>

#include <sys/mman.h>

namespace menps {
namespace mefdn {

class mapped_memory
{
public:
    mapped_memory()
        : ptr_{nullptr}
        , size_in_bytes_{0} { }
    
    mapped_memory(void* const ptr, const mefdn::size_t size_in_bytes)
        : ptr_{ptr}
        , size_in_bytes_{size_in_bytes} { }
    
    mapped_memory(const mapped_memory&) = delete;
    mapped_memory& operator = (const mapped_memory&) = delete;
    
    mapped_memory(mapped_memory&& other)
        : ptr_{nullptr}
        , size_in_bytes_{0}
    {
        *this = mefdn::move(other);
    }
    
    mapped_memory& operator = (mapped_memory&& other)
    {
        unmap();
        
        mefdn::swap(this->ptr_, other.ptr_);
        mefdn::swap(this->size_in_bytes_, other.size_in_bytes_);
        
        return *this;
    }
    
    ~mapped_memory()
    {
        unmap();
    }
    
    void* get() const noexcept
    {
        MEFDN_ASSERT(ptr_ != nullptr);
        return ptr_;
    }
    
    static mapped_memory map(
        void* const             addr
    ,   const mefdn::size_t    length
    ,   const int               prot
    ,   const int               flags
    ,   const int               fd
    ,   const off_t             offset
    )
    {
        void* const ret = mmap(addr, length, prot, flags, fd, offset);
        
        if (ret == MAP_FAILED)
        {
            MEFDN_LOG_WARN(
                "msg:mmap() failed.\t"
                "addr:{:x}\t"
                "length:{}\t"
                "prot:{}\t"
                "flags:{}\t"
                "fd:{}\t"
                "offset:{}"
            ,   reinterpret_cast<mefdn::uintptr_t>(addr)
            ,   length
            ,   prot
            ,   flags
            ,   fd
            ,   offset
            );
            
            throw std::bad_alloc{};
        }
        
        MEFDN_LOG_VERBOSE(
            "msg:Called mmap().\t"
            "ret:{:x}\t"
            "addr:{:x}\t"
            "length:{}\t"
            "prot:{}\t"
            "flags:{}\t"
            "fd:{}\t"
            "offset:{}"
        ,   reinterpret_cast<mefdn::uintptr_t>(ret)
        ,   reinterpret_cast<mefdn::uintptr_t>(addr)
        ,   length
        ,   prot
        ,   flags
        ,   fd
        ,   offset
        );
        
        return mapped_memory(ret, length);
    }
    
private:
    void unmap()
    {
        if (ptr_ != nullptr)
        {
            MEFDN_MAYBE_UNUSED
            const auto ret = munmap(ptr_, size_in_bytes_);
            
            MEFDN_LOG_VERBOSE(
                "msg:Called munmap().\t"
                "ret:{}\t"
                "ptr:{:x}\t"
                "size_in_bytes:{}"
            ,   ret
            ,   reinterpret_cast<mefdn::uintptr_t>(ptr_)
            ,   size_in_bytes_
            );
            
            ptr_ = nullptr;
            size_in_bytes_ = 0;
        }
    }
    
    void*           ptr_;
    mefdn::size_t  size_in_bytes_;
};

} // namespace mefdn
} // namespace menps


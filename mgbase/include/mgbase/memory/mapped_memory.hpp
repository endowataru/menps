
#pragma once

#include <mgbase/utility.hpp>

#include <sys/mman.h>

namespace mgbase {

class mapped_memory
{
public:
    mapped_memory()
        : ptr_{MGBASE_NULLPTR}
        , size_in_bytes_{0} { }
    
    mapped_memory(void* const ptr, const mgbase::size_t size_in_bytes)
        : ptr_{ptr}
        , size_in_bytes_{size_in_bytes} { }
    
    mapped_memory(const mapped_memory&) = delete;
    mapped_memory& operator = (const mapped_memory&) = delete;
    
    mapped_memory(mapped_memory&& other)
        : ptr_{MGBASE_NULLPTR}
        , size_in_bytes_{0}
    {
        *this = mgbase::move(other);
    }
    
    mapped_memory& operator = (mapped_memory&& other)
    {
        unmap();
        
        mgbase::swap(this->ptr_, other.ptr_);
        mgbase::swap(this->size_in_bytes_, other.size_in_bytes_);
        
        return *this;
    }
    
    ~mapped_memory()
    {
        unmap();
    }
    
    void* get() const MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(ptr_ != MGBASE_NULLPTR);
        return ptr_;
    }
    
    static mapped_memory map(
        void* const             addr
    ,   const mgbase::size_t    length
    ,   const int               prot
    ,   const int               flags
    ,   const int               fd
    ,   const off_t             offset
    )
    {
        void* const ret = mmap(addr, length, prot, flags, fd, offset);
        
        if (ret == MAP_FAILED)
        {
            MGBASE_LOG_WARN(
                "msg:mmap() failed.\t"
                "addr:{:x}\t"
                "length:{}\t"
                "prot:{}\t"
                "flags:{}\t"
                "fd:{}\t"
                "offset:{}"
            ,   reinterpret_cast<mgbase::uintptr_t>(addr)
            ,   length
            ,   prot
            ,   flags
            ,   fd
            ,   offset
            );
            
            throw std::bad_alloc{};
        }
        
        MGBASE_LOG_VERBOSE(
            "msg:Called mmap().\t"
            "ret:{:x}\t"
            "addr:{:x}\t"
            "length:{}\t"
            "prot:{}\t"
            "flags:{}\t"
            "fd:{}\t"
            "offset:{}"
        ,   reinterpret_cast<mgbase::uintptr_t>(ret)
        ,   reinterpret_cast<mgbase::uintptr_t>(addr)
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
        if (ptr_ != MGBASE_NULLPTR)
        {
            MGBASE_UNUSED
            const auto ret = munmap(ptr_, size_in_bytes_);
            
            MGBASE_LOG_VERBOSE(
                "msg:Called munmap().\t"
                "ret:{}\t"
                "ptr:{:x}\t"
                "size_in_bytes:{}"
            ,   ret
            ,   reinterpret_cast<mgbase::uintptr_t>(ptr_)
            ,   size_in_bytes_
            );
            
            ptr_ = MGBASE_NULLPTR;
            size_in_bytes_ = 0;
        }
    }
    
    void*           ptr_;
    mgbase::size_t  size_in_bytes_;
};

} // namespace mgbase


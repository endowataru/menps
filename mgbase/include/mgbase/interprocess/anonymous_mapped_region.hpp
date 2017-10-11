
#pragma once

#include <mgbase/lang.hpp>

#include <sys/mman.h> // mmap

namespace mgbase {

struct anonymous_mapped_region_error { };

class anonymous_mapped_region
{
public:
    explicit anonymous_mapped_region(mgbase::size_t size)
    {
        ptr_ = mmap(
            MGBASE_NULLPTR              // addr
        ,   size                        // length
        ,   PROT_READ | PROT_WRITE      // prot
        ,   MAP_SHARED | MAP_ANONYMOUS  // flags
        ,   -1                          // fd
        ,   0                           // offset
        );
        
        if (ptr_ == MAP_FAILED)
            throw anonymous_mapped_region_error();
    }
    
    ~anonymous_mapped_region()
    {
        // do nothing in POSIX
    }
    
    MGBASE_ALWAYS_INLINE mgbase::size_t get_size() const MGBASE_NOEXCEPT {
        return size_;
    }
    
    MGBASE_ALWAYS_INLINE void* get_address() const MGBASE_NOEXCEPT {
        return ptr_;
    }
    
private:
    void* ptr_;
    mgbase::size_t size_;
};

} // namespace mgbase


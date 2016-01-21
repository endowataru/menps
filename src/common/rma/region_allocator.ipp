
#pragma once

#include "region_allocator.hpp"
#include <mgcom/rma/address.hpp>
#include "./malloc.h"
#include <mgbase/memory/aligned_alloc.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace rma {
namespace untyped {

namespace /*untyped*/ {

class region_allocator
{
    // FIXME: adjustable buffer size
    static const index_t total_region_size = 256 << 20;
    
public:
    void initialize()
    {
        // Allocate a huge buffer.
        void* const ptr = mgbase::aligned_alloc(buffer_alignment, total_region_size);
        
        // Register it to the RDMA engine.
        region_ = register_region(ptr, total_region_size);
        
        // Prepare dlmalloc.
        ms_ = create_mspace_with_base(ptr, total_region_size, 1);
    }
    
    void finalize()
    {
        void* const ptr = to_raw_pointer(region_);
        
        deregister_region(region_);
        
        destroy_mspace(ms_);
    }
    
    registered_buffer allocate(index_t size_in_bytes)
    {
        // FIXME : Multithreading problem
        
        void* const ptr = mspace_malloc(ms_, size_in_bytes);
        
        const local_address base = to_address(region_);
        const index_t diff = static_cast<index_t>(
            reinterpret_cast<mgbase::uintptr_t>(ptr) - reinterpret_cast<mgbase::uintptr_t>(to_raw_pointer(base))
        );
        
        const local_address addr = advanced(base, diff);
        const registered_buffer result = { addr };
        
        MGBASE_LOG_DEBUG(
            "msg:Allocated from buffer.\t"
            "ptr:{:x}\tpointer:{:x}\tkey_info:{:x}\tregion_info:{:x}\t"
            "offset:{:x}\tsize_in_bytes:{}"
        ,   reinterpret_cast<mgbase::uintptr_t>(ptr)
        ,   reinterpret_cast<mgbase::uintptr_t>(addr.region.key.pointer)
        ,   addr.region.key.info
        ,   addr.region.info
        ,   addr.offset
        ,   size_in_bytes
        );
        
        return result;
    }
    
    void deallocate(const registered_buffer& buf)
    {
        void* const ptr = to_raw_pointer(buf);
        mspace_free(ms_, ptr);
    }
    

private:
    local_region region_;
    mspace ms_;
};

} // untyped namespace

} // namespace untyped
} // namespace rma
} // namespace mgcom


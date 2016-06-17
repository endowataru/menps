
#pragma once

#include "region_allocator.hpp"
#include <mgcom/rma/registration.hpp>
#include "./malloc.h"
#include <mgbase/memory/aligned_alloc.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace rma {
namespace untyped {

namespace /*unnamed*/ {

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
        
        MGBASE_LOG_DEBUG("msg:Initialized RMA region allocator.");
    }
    
    void finalize()
    {
        deregister_region(region_);
        
        destroy_mspace(ms_);
        
        MGBASE_LOG_DEBUG("msg:Finalized RMA region allocator.");
    }
    
    registered_buffer allocate(index_t size_in_bytes)
    {
        // TODO: reduce multithreading contentions
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        
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
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        
        void* const ptr = to_raw_pointer(buf);
        mspace_free(ms_, ptr);
        
        MGBASE_LOG_DEBUG(
            "msg:Deallocated buffer.\tptr:{:x}"
        ,   reinterpret_cast<mgbase::uintptr_t>(ptr)
        );
    }

private:
    mgbase::spinlock lock_;
    local_region region_;
    mspace ms_;
};

} // unnamed namespace

} // namespace untyped
} // namespace rma
} // namespace mgcom


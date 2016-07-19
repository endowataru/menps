
#include "default_allocator.hpp"
#include <mgcom/rma/registration.hpp>
#include "./malloc.h"
#include <mgbase/memory/aligned_alloc.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

class default_allocator
    : public allocator
{
    // FIXME: adjustable buffer size
    static const index_t total_region_size = 256 << 20;
    
public:
    explicit default_allocator(registrator& reg)
        : reg_(reg)
    {
        // Allocate a huge buffer.
        void* const ptr = mgbase::aligned_alloc(buffer_alignment, total_region_size);
        
        // Register it to the RDMA engine.
        region_ = reg.register_region(untyped::register_region_params{ptr, total_region_size});
        
        // Prepare dlmalloc.
        ms_ = create_mspace_with_base(ptr, total_region_size, 1);
        
        MGBASE_LOG_DEBUG("msg:Initialized default allocator for RMA.");
        
        from_ = reinterpret_cast<mgbase::uintptr_t>(ptr);
        to_ = reinterpret_cast<mgbase::uintptr_t>(ptr) + total_region_size;
    }
    
    virtual ~default_allocator()
    {
        reg_.deregister_region(untyped::deregister_region_params{region_});
        
        destroy_mspace(ms_);
        
        MGBASE_LOG_DEBUG("msg:Finalized default allocator for RMA.");
    }
    
    default_allocator(const default_allocator&) = delete;
    default_allocator& operator = (const default_allocator&) = delete;
    
    virtual untyped::registered_buffer allocate(const index_t size_in_bytes) MGBASE_OVERRIDE
    {
        // TODO: reduce multithreading contentions
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        
        void* const ptr = mspace_malloc(ms_, size_in_bytes);
        
        MGBASE_ASSERT(ptr != MGBASE_NULLPTR);
        
        MGBASE_ASSERT(from_ <= reinterpret_cast<mgbase::uintptr_t>(ptr));
        MGBASE_ASSERT(reinterpret_cast<mgbase::uintptr_t>(ptr) < to_);
        
        const untyped::local_address base = untyped::to_address(region_);
        const index_t diff = static_cast<index_t>(
            reinterpret_cast<mgbase::uintptr_t>(ptr) -
            reinterpret_cast<mgbase::uintptr_t>(untyped::to_raw_pointer(base))
        );
        
        const untyped::local_address addr = untyped::advanced(base, diff);
        const untyped::registered_buffer result = { addr };
        
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
    
    virtual void deallocate(const untyped::registered_buffer& buf) MGBASE_OVERRIDE
    {
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        
        void* const ptr = untyped::to_raw_pointer(buf);
        mspace_free(ms_, ptr);
        
        MGBASE_LOG_DEBUG(
            "msg:Deallocated buffer.\tptr:{:x}"
        ,   reinterpret_cast<mgbase::uintptr_t>(ptr)
        );
    }

private:
    registrator& reg_;
    
    mgbase::spinlock lock_;
    untyped::local_region region_;
    mspace ms_;
    mgbase::uintptr_t from_;
    mgbase::uintptr_t to_;
};

} // unnamed namespace

default_allocator_ptr make_default_allocator(registrator& reg)
{
    return mgbase::make_unique<default_allocator>(reg);
}

} // namespace rma
} // namespace mgcom


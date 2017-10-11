
#include "default_allocator.hpp"
#include <mgcom/rma/registration.hpp>
#include <mgbase/external/malloc.h>
#include <mgbase/memory/aligned_alloc.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/logging/logger.hpp>
#include <mgbase/arithmetic.hpp>
#include <mgbase/shared_ptr.hpp>
#include <unordered_map>

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

class default_allocator
    : public allocator
{
    class allocated_region
    {
    public:
        allocated_region(
            registrator&    reg
        ,   const index_t   region_size
        )
            : reg_(reg)
        {
            // Allocate a huge buffer.
            void* const ptr = mgbase::aligned_alloc(buffer_alignment, region_size);
            
            // Register it to the RDMA engine.
            region_ = reg.register_region(untyped::register_region_params{ptr, region_size});
            
            // Prepare dlmalloc.
            ms_ = create_mspace_with_base(ptr, region_size, 1);
            
            from_ = reinterpret_cast<mgbase::uintptr_t>(ptr);
            to_ = reinterpret_cast<mgbase::uintptr_t>(ptr) + region_size;
        }
        
        ~allocated_region()
        {
            reg_.deregister_region(untyped::deregister_region_params{region_});
            
            destroy_mspace(ms_);
        }
        
        bool try_allocate(const index_t size_in_bytes, untyped::registered_buffer* buffer_result)
        {
            void* const ptr = mspace_malloc(ms_, size_in_bytes);
            if (ptr == MGBASE_NULLPTR)
            {
                MGBASE_LOG_DEBUG(
                    "msg:Failed to allocate memory for RMA.\t"
                    "size_in_bytes:{}"
                ,   size_in_bytes
                );
                
                return false;
            }
            
            MGBASE_ASSERT(ptr != MGBASE_NULLPTR);
            
            MGBASE_ASSERT(from_ <= reinterpret_cast<mgbase::uintptr_t>(ptr));
            
            if (reinterpret_cast<mgbase::uintptr_t>(ptr) + size_in_bytes > to_)
            {
                mspace_free(ms_, ptr);
                return false;
            }
            
            MGBASE_ASSERT(reinterpret_cast<mgbase::uintptr_t>(ptr) + size_in_bytes <= to_);
            
            const untyped::local_address base = untyped::to_address(region_);
            const intptr_t diff =
                reinterpret_cast<mgbase::intptr_t>(ptr) -
                reinterpret_cast<mgbase::intptr_t>(untyped::to_raw_pointer(base));
            
            const untyped::local_address addr = untyped::advanced(base, diff);
            
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
            
            const untyped::registered_buffer result = { addr };
            *buffer_result = result;
            
            return true;
        }
        
        void* get_pointer() const MGBASE_NOEXCEPT {
            return untyped::to_raw_pointer(region_);
        }
        
        void deallocate(const untyped::registered_buffer& buf)
        {
            void* const ptr = untyped::to_raw_pointer(buf);
            mspace_free(ms_, ptr);
            
            MGBASE_LOG_DEBUG(
                "msg:Deallocated buffer.\tptr:{:x}"
            ,   reinterpret_cast<mgbase::uintptr_t>(ptr)
            );
        }
        
    private:
        registrator&            reg_;
        untyped::local_region   region_;
        mspace                  ms_;
        mgbase::uintptr_t       from_;
        mgbase::uintptr_t       to_;
    };
    
public:
    explicit default_allocator(
        registrator&    reg
    ,   const index_t   total_size
    ,   const index_t   region_size
    )
    {
        const index_t num_regions = mgbase::roundup_divide(total_size, region_size);
        
        for (index_t i = 0; i < num_regions; ++i) {
            regs_.push_back(mgbase::shared_ptr<allocated_region>(
                new allocated_region(reg, region_size)
            ));
            
            const auto reg_ptr = regs_[i]->get_pointer();
            reg_ptr_to_index_[reg_ptr] = i;
        }
        
        MGBASE_LOG_DEBUG("msg:Initialized default allocator for RMA.");
    }
    
    virtual ~default_allocator()
    {
        MGBASE_LOG_DEBUG("msg:Finalizing default allocator for RMA.");
    }
    
    default_allocator(const default_allocator&) = delete;
    default_allocator& operator = (const default_allocator&) = delete;
    
    virtual untyped::registered_buffer allocate(const index_t size_in_bytes) MGBASE_OVERRIDE
    {
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        
        untyped::registered_buffer buf;
        
        // TODO: linear search
        MGBASE_RANGE_BASED_FOR(auto&& reg, regs_) {
            if (reg->try_allocate(size_in_bytes, &buf)) {
                return buf;
            }
        }
        
        throw std::bad_alloc{};
    }
    
    virtual void deallocate(const untyped::registered_buffer& buf) MGBASE_OVERRIDE
    {
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        
        const auto& addr = buf.addr;
        
        const auto reg_ptr = addr.region.key.pointer;
        
        MGBASE_LOG_DEBUG(
            "msg:Deallocating buffer.\t"
            "ptr:{:x}\tkey_info:{:x}\tregion_info:{:x}\t"
            "offset:{:x}\treg_ptr:{}"
        ,   reinterpret_cast<mgbase::uintptr_t>(addr.region.key.pointer)
        ,   addr.region.key.info
        ,   addr.region.info
        ,   addr.offset
        ,   reg_ptr
        );
        
        const auto reg_index = reg_ptr_to_index_[reg_ptr];
        
        regs_[reg_index]->deallocate(buf);
    }

private:
    mgbase::spinlock lock_;
    
    std::vector<mgbase::shared_ptr<allocated_region>> regs_;
    std::unordered_map<void*, index_t> reg_ptr_to_index_;
};

} // unnamed namespace

default_allocator_ptr make_default_allocator(
    registrator&    reg
,   const index_t   total_size
,   const index_t   region_size
) {
    return mgbase::make_unique<default_allocator>(reg, total_size, region_size);
}

} // namespace rma
} // namespace mgcom


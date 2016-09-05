
#pragma once

#include "completer.hpp"
#include <mgcom/rma/allocation.hpp>

namespace mgcom {
namespace ibv {

class atomic_buffer
{
public:
    atomic_buffer(rma::allocator& alloc, const mgbase::size_t max_num_completions)
        : alloc_(alloc)
        , entries_(max_num_completions)
        , buf_{rma::allocate<rma::atomic_default_t>(alloc, max_num_completions)}
    {
        for (mgbase::size_t i = 0; i < max_num_completions; ++i)
            entries_[i].src = &buf_[static_cast<mgbase::ptrdiff_t>(i)];
    }
    
    ~atomic_buffer()
    {
        rma::deallocate(alloc_, buf_);
    }
    
    atomic_buffer(const atomic_buffer&) = delete;
    atomic_buffer& operator = (const atomic_buffer&) = delete;
    
    struct make_notification_result
    {
        mgbase::callback<void ()>                       on_complete;
        mgcom::rma::local_ptr<rma::atomic_default_t>    buf_lptr;
    };
    
    // write
    make_notification_result make_notification_write(
        const mgbase::uint64_t              wr_id
    ,   const mgbase::callback<void ()>&    on_complete
    ) {
        make_notification_base(wr_id, on_complete, MGBASE_NULLPTR);
        
        return {
            write_callback{ &entries_[wr_id] }
        ,   buf_at(wr_id)
        };
    }
    
    // read
    make_notification_result make_notification_read(
        const mgbase::uint64_t              wr_id
    ,   const mgbase::callback<void ()>&    on_complete
    ,   mgbase::uint64_t* const             dest_ptr
    ) {
        make_notification_base(wr_id, on_complete, dest_ptr);
        
        return {
            read_callback{ &entries_[wr_id] }
        ,   buf_at(wr_id)
        };
    }
    
    // CAS, FAA
    make_notification_result make_notification_atomic(
        const mgbase::uint64_t              wr_id
    ,   const mgbase::callback<void ()>&    on_complete
    ,   mgbase::uint64_t* const             dest_ptr
    ) {
        make_notification_base(wr_id, on_complete, dest_ptr);
        
        return {
            atomic_callback{ &entries_[wr_id] }
        ,   buf_at(wr_id)
        };
    }
    
private:
    struct entry
    {
        mgbase::callback<void ()>       on_complete;
        rma::atomic_default_t*  src;
        rma::atomic_default_t*  dest;
    };
    
    struct write_callback
    {
        entry* e;
        
        void operator() () const
        {
            // do nothing
            
            // Execute the callback.
            e->on_complete();
        }
    };
    
    struct read_callback
    {
        entry* e;
        
        void operator() () const
        {
            const mgbase::uint64_t val = *e->src;
            
            *e->dest = val;
            
            // Execute the callback.
            e->on_complete();
        }
    };
    
    struct atomic_callback
    {
        entry* e;
        
        void operator() () const
        {
            mgbase::uint64_t orig_val = *e->src;
            
            #ifdef MGCOM_IBV_MASKED_ATOMICS_SUPPORTED
                // Big-endian to little-endian
                mgbase::uint64_t val = 0;
                for (mgbase::size_t i = 0; i < 8; ++i)
                    val |= ((orig_val >> (8 * (7 - i))) & 0xFF) << (8 * i);
            #else
                const mgbase::uint64_t val = orig_val;
            #endif
            
            *e->dest = val;
            
            // Execute the callback.
            e->on_complete();
        }
    };
    
private:
    rma::local_ptr<rma::atomic_default_t> buf_at(const mgbase::size_t index)
    {
        return buf_ + static_cast<mgbase::ptrdiff_t>(index);
    }
    
    void make_notification_base(
        const mgbase::uint64_t              wr_id
    ,   const mgbase::callback<void ()>&    on_complete
    ,   mgbase::uint64_t* const             dest_ptr
    ) {
        entry& e = entries_[wr_id];
        e.on_complete = on_complete;
        e.dest = dest_ptr;
    }
    
    rma::allocator& alloc_;
    std::vector<entry> entries_;
    rma::local_ptr<rma::atomic_default_t> buf_;
};

} // namespace ibv
} // namespace mgcom


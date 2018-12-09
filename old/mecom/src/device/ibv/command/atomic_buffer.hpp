
#pragma once

#include <menps/mecom/rma/allocation.hpp>
#include "device/ibv/native/endpoint.hpp"

namespace menps {
namespace mecom {
namespace ibv {

class atomic_buffer
{
public:
    struct config
    {
        rma::allocator&     alloc;
        mefdn::size_t      max_num_completions;
        bool                reply_be;
    };
    
    explicit atomic_buffer(const config& conf)
        : conf_(conf)
        , entries_(conf.max_num_completions)
        , buf_(rma::allocate<rma::atomic_default_t>(conf.alloc, conf.max_num_completions))
    {
        for (mefdn::size_t i = 0; i < this->conf_.max_num_completions; ++i)
            entries_[i].src = &buf_[static_cast<mefdn::ptrdiff_t>(i)];
    }
    
    ~atomic_buffer()
    {
        rma::deallocate(conf_.alloc, buf_);
    }
    
    atomic_buffer(const atomic_buffer&) = delete;
    atomic_buffer& operator = (const atomic_buffer&) = delete;
    
    struct make_notification_result
    {
        mefdn::callback<void ()>                       on_complete;
        mecom::rma::local_ptr<rma::atomic_default_t>    buf_lptr;
    };
    
    // write
    make_notification_result make_notification_write(
        const mefdn::uint64_t              wr_id
    ,   const mefdn::callback<void ()>&    on_complete
    ) {
        make_notification_base(wr_id, on_complete, nullptr);
        
        return {
            write_callback{ &entries_[wr_id] }
        ,   buf_at(wr_id)
        };
    }
    
    // read
    make_notification_result make_notification_read(
        const mefdn::uint64_t              wr_id
    ,   const mefdn::callback<void ()>&    on_complete
    ,   mefdn::uint64_t* const             dest_ptr
    ) {
        make_notification_base(wr_id, on_complete, dest_ptr);
        
        return {
            read_callback{ &entries_[wr_id] }
        ,   buf_at(wr_id)
        };
    }
    
    // CAS, FAA
    make_notification_result make_notification_atomic(
        const mefdn::uint64_t              wr_id
    ,   const mefdn::callback<void ()>&    on_complete
    ,   mefdn::uint64_t* const             dest_ptr
    ) {
        make_notification_base(wr_id, on_complete, dest_ptr);
        
        return {
            atomic_callback{ this, &entries_[wr_id] }
        ,   buf_at(wr_id)
        };
    }
    
private:
    struct entry
    {
        mefdn::callback<void ()>   on_complete;
        rma::atomic_default_t*      src;
        rma::atomic_default_t*      dest;
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
            const mefdn::uint64_t val = *e->src;
            
            *e->dest = val;
            
            // Execute the callback.
            e->on_complete();
        }
    };
    
    struct atomic_callback
    {
        atomic_buffer*  self;
        entry*          e;
        
        void operator() () const
        {
            mefdn::uint64_t orig_val = *e->src;
            
            mefdn::uint64_t val = 0;
            if (self->conf_.reply_be) {
                // Big-endian to little-endian
                for (mefdn::size_t i = 0; i < 8; ++i)
                    val |= ((orig_val >> (8 * (7 - i))) & 0xFF) << (8 * i);
            } else {
                val = orig_val;
            }
            
            *e->dest = val;
            
            // Execute the callback.
            e->on_complete();
        }
    };
    
private:
    rma::local_ptr<rma::atomic_default_t> buf_at(const mefdn::size_t index)
    {
        return buf_ + static_cast<mefdn::ptrdiff_t>(index);
    }
    
    void make_notification_base(
        const mefdn::uint64_t              wr_id
    ,   const mefdn::callback<void ()>&    on_complete
    ,   mefdn::uint64_t* const             dest_ptr
    ) {
        auto& e = entries_[wr_id];
        e.on_complete = on_complete;
        e.dest = dest_ptr;
    }
    
    const config                            conf_;
    std::vector<entry>                      entries_;
    rma::local_ptr<rma::atomic_default_t>   buf_;
};

} // namespace ibv
} // namespace mecom
} // namespace menps


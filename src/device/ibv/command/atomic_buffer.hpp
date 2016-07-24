
#pragma once

#include "completer.hpp"
#include <mgcom/rma/allocation.hpp>

namespace mgcom {
namespace ibv {

class atomic_buffer
{
    static const mgbase::size_t max_num_completions = completer::max_num_completions;
    
public:
    explicit atomic_buffer(rma::allocator& alloc)
        : alloc_(alloc)
        , entries_{new entry[max_num_completions]}
        , buf_{rma::allocate<rma::atomic_default_t>(alloc, max_num_completions)}
    {
        for (mgbase::size_t i = 0; i < max_num_completions; ++i)
            entries_[i].src = &buf_[i];
    }
    
    ~atomic_buffer() {
        rma::deallocate(alloc_, buf_);
        entries_.reset();
    }
    
    atomic_buffer(const atomic_buffer&) = delete;
    atomic_buffer& operator = (const atomic_buffer&) = delete;
    
    struct make_notification_result
    {
        mgbase::operation                               on_complete;
        mgcom::rma::local_ptr<rma::atomic_default_t>    buf_lptr;
    };
    
    // write
    make_notification_result make_notification_write(
        const mgbase::uint64_t      wr_id
    ,   const mgbase::operation&    on_complete
    ) {
        make_notification_base(wr_id, on_complete, MGBASE_NULLPTR);
        
        return {
            mgbase::make_operation_call(
                mgbase::make_callback_function(
                    mgbase::bind1st_of_2(
                        MGBASE_MAKE_INLINED_FUNCTION(atomic_buffer::handler_write)
                    ,   mgbase::wrap_reference(entries_[wr_id])
                    )
                )
            )
        ,   buf_ + wr_id
        };
    }
    
    // read
    make_notification_result make_notification_read(
        const mgbase::uint64_t      wr_id
    ,   const mgbase::operation&    on_complete
    ,   mgbase::uint64_t* const     dest_ptr
    ) {
        make_notification_base(wr_id, on_complete, dest_ptr);
        
        return {
            mgbase::make_operation_call(
                mgbase::make_callback_function(
                    mgbase::bind1st_of_2(
                        MGBASE_MAKE_INLINED_FUNCTION(atomic_buffer::handler_read)
                    ,   mgbase::wrap_reference(entries_[wr_id])
                    )
                )
            )
        ,   buf_ + wr_id
        };
    }
    
    // CAS, FAA
    make_notification_result make_notification_atomic(
        const mgbase::uint64_t      wr_id
    ,   const mgbase::operation&    on_complete
    ,   mgbase::uint64_t* const     dest_ptr
    ) {
        make_notification_base(wr_id, on_complete, dest_ptr);
        
        return {
            mgbase::make_operation_call(
                mgbase::make_callback_function(
                    mgbase::bind1st_of_2(
                        MGBASE_MAKE_INLINED_FUNCTION(atomic_buffer::handler_atomic)
                    ,   mgbase::wrap_reference(entries_[wr_id])
                    )
                )
            )
        ,   buf_ + wr_id
        };
    }
    
private:
    struct entry
    {
        mgbase::operation       on_complete;
        rma::atomic_default_t*  src;
        rma::atomic_default_t*  dest;
    };
    
    static void handler_write(const entry& e, const mgbase::operation& /*ignored*/)
    {
        // do nothing
        
        mgbase::execute(e.on_complete);
    }
    
    static void handler_read(const entry& e, const mgbase::operation& /*ignored*/)
    {
        const mgbase::uint64_t val = *e.src;
        
        *e.dest = val;
        
        mgbase::execute(e.on_complete);
    }
    
    static void handler_atomic(const entry& e, const mgbase::operation& /*ignored*/)
    {
        mgbase::uint64_t orig_val = *e.src;
        
        #ifdef MGCOM_IBV_MASKED_ATOMICS_SUPPORTED
            // Big-endian to little-endian
            mgbase::uint64_t val = 0;
            for (mgbase::size_t i = 0; i < 8; ++i)
                val |= ((orig_val >> (8 * (7 - i))) & 0xFF) << (8 * i);
        #else
            const mgbase::uint64_t val = orig_val;
        #endif
        
        *e.dest = val;
        
        mgbase::execute(e.on_complete);
    }
    
private:
    void make_notification_base(
        const mgbase::uint64_t      wr_id
    ,   const mgbase::operation&    on_complete
    ,   mgbase::uint64_t* const     dest_ptr
    ) {
        entry& e = entries_[wr_id];
        e.on_complete = on_complete;
        e.dest = dest_ptr;
    }
    
    rma::allocator& alloc_;
    mgbase::scoped_ptr<entry []> entries_;
    rma::local_ptr<rma::atomic_default_t> buf_;
};

} // namespace ibv
} // namespace mgcom


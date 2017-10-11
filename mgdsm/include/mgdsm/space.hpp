
#pragma once

#include <mgdsm/segment_ref.hpp>
#include <mgbase/callback.hpp>

namespace mgdsm {

class space
{
public:
    virtual ~space() /*noexcept*/ = default;
    
    virtual segment_ref make_segment(
        mgbase::size_t  size_in_bytes
    ,   mgbase::size_t  page_size_in_bytes
    ,   mgbase::size_t  block_size_in_bytes
    ) = 0;
    
    virtual void read_barrier() = 0;
    
    virtual void write_barrier() = 0;
    
    virtual void async_read_barrier(mgbase::callback<void ()>) = 0;
    
    virtual void async_write_barrier(mgbase::callback<void ()>) = 0;
    
    virtual void enable_on_this_thread() = 0;
    
    virtual void disable_on_this_thread() = 0;
    
    virtual void pin(void*, mgbase::size_t) = 0;
    
    virtual void unpin(void*, mgbase::size_t) = 0;
};

/*
    Notes: allocation is not provided here.
*/

} // namespace mgdsm


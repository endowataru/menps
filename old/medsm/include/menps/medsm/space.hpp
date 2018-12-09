
#pragma once

#include <menps/medsm/segment_ref.hpp>
#include <menps/mefdn/callback.hpp>

namespace menps {
namespace medsm {

class space
{
public:
    virtual ~space() /*noexcept*/ = default;
    
    virtual segment_ref make_segment(
        mefdn::size_t  size_in_bytes
    ,   mefdn::size_t  page_size_in_bytes
    ,   mefdn::size_t  block_size_in_bytes
    ) = 0;
    
    virtual void read_barrier() = 0;
    
    virtual void write_barrier() = 0;
    
    virtual void async_read_barrier(mefdn::callback<void ()>) = 0;
    
    virtual void async_write_barrier(mefdn::callback<void ()>) = 0;
    
    virtual void enable_on_this_thread() = 0;
    
    virtual void disable_on_this_thread() = 0;
    
    virtual void pin(void*, mefdn::size_t) = 0;
    
    virtual void unpin(void*, mefdn::size_t) = 0;
};

/*
    Notes: allocation is not provided here.
*/

} // namespace medsm
} // namespace menps


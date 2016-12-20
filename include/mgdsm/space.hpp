
#pragma once

#include <mgdsm/segment_ref.hpp>

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
    
    #if 0
    virtual void fetch(void* ptr, mgbase::size_t size_in_bytes) = 0;
    
    virtual void touch(void* ptr, mgbase::size_t size_in_bytes) = 0;
    
    virtual void pin(void* ptr, mgbase::size_t size_in_bytes) = 0;
    
    virtual void unpin(void* ptr, mgbase::size_t size_in_bytes) = 0;
    
    virtual void write_barrier() = 0;
    
    virtual void read_barrier() = 0;
    #endif
    
    virtual void enable_on_this_thread() = 0;
    
    virtual void disable_on_this_thread() = 0;
};

/*
    Notes: allocation is not provided here.
*/

} // namespace mgdsm


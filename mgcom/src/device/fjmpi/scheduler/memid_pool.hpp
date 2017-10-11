
#pragma once

#include "device/fjmpi/fjmpi.hpp"
#include <mgbase/container/circular_buffer.hpp>
#include <mgbase/threading/spinlock.hpp>

namespace mgcom {
namespace fjmpi {

class memid_pool
{
public:
    memid_pool()
    {
        for (int memid = 0; memid < constants::max_memid_count; memid++)
            buf_.push_back(memid);
    }
    
    memid_pool(const memid_pool&) = delete;
    memid_pool& operator = (const memid_pool&) = delete;
    
    bool try_allocate(int* const memid_result)
    {
        mgbase::lock_guard<lock_type> lc{ lock_ };
        
        if (buf_.empty())
            return false;
        
        *memid_result = buf_.front();
        buf_.pop_front();
        return true;
    }
    
    void deallocate(int memid)
    {
        mgbase::lock_guard<lock_type> lc{ lock_ };
        
        buf_.push_back(memid);
    }
    
private:
    typedef mgbase::spinlock    lock_type;
    
    typedef mgbase::static_circular_buffer<int, constants::max_memid_count>
        buffer_type;
    
    lock_type lock_;
    buffer_type buf_;
};

} // namespace fjmpi
} // namespace mgcom


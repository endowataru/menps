
#pragma once

#include "device/fjmpi/fjmpi.hpp"
#include <menps/mefdn/container/circular_buffer.hpp>
#include <menps/mefdn/threading/spinlock.hpp>

namespace menps {
namespace mecom {
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
        mefdn::lock_guard<lock_type> lc{ lock_ };
        
        if (buf_.empty())
            return false;
        
        *memid_result = buf_.front();
        buf_.pop_front();
        return true;
    }
    
    void deallocate(int memid)
    {
        mefdn::lock_guard<lock_type> lc{ lock_ };
        
        buf_.push_back(memid);
    }
    
private:
    typedef mefdn::spinlock    lock_type;
    
    typedef mefdn::static_circular_buffer<int, constants::max_memid_count>
        buffer_type;
    
    lock_type lock_;
    buffer_type buf_;
};

} // namespace fjmpi
} // namespace mecom
} // namespace menps


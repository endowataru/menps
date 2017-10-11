
#pragma once

#include "synchronic.hpp"

namespace mgbase {

class sync_flag
{
public:
    sync_flag()
        : flag_{false} { }
    
    sync_flag(const sync_flag&) = delete;
    
    sync_flag& operator = (const sync_flag&) = delete;
    
    void notify()
    {
        sync_.notify(flag_, true);
    }
    
    void wait()
    {
        sync_.expect(flag_, true);
    }
    
private:
    mgbase::atomic<bool>    flag_;
    synchronic<bool>        sync_;
};

} // namespace mgbase


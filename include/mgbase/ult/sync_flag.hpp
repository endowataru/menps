
#pragma once

#include "synchronic.hpp"

namespace mgbase {
namespace ult {

class sync_flag
{
public:
    sync_flag()
        : flag_(MGBASE_ATOMIC_VAR_INIT(false)) { }
    
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
    mgbase::atomic<bool>        flag_;
    synchronic<bool>    sync_;
};

} // namespace ult
} // namespace mgbase


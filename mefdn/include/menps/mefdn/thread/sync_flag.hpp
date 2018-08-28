
#pragma once

#include "synchronic.hpp"

namespace menps {
namespace mefdn {

// TODO: Marking this class produces lots of unimportant warnings.
//       This class is no longer maintained, though.
class /*MEFDN_DEPRECATED*/ sync_flag
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
    mefdn::atomic<bool>    flag_;
    synchronic<bool>        sync_;
};

} // namespace mefdn
} // namespace menps


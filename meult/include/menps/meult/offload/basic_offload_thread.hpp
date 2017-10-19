
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/atomic.hpp>
#include <menps/mefdn/crtp_base.hpp>
#include <menps/mefdn/profiling/clock.hpp>

namespace menps {
namespace meult {

template <typename Policy>
class basic_offload_thread
{
    MEFDN_POLICY_BASED_CRTP(Policy)
    
protected:
    basic_offload_thread()
        : finished_{false}
    { }
    
    bool dequeue_some()
    {
        auto& self = this->derived();
        
        auto t = self.try_dequeue();
        if (!t.valid()) {
            return false;
        }
        
        mefdn::size_t n = 0;
        
        // TODO: exception safety
        
        MEFDN_RANGE_BASED_FOR(auto&& cmd, t)
        {
            if (!self.try_execute(
                mefdn::forward<decltype(cmd)>(cmd)
            )) {
                break;
            }

            ++n;
        }
        
        t.commit(n);
        
        if (n > 0) {
            MEFDN_LOG_DEBUG(
                "msg:Dequeued IBV requests.\t"
                "n:{}"
            ,   n
            );
        }
        else {
            MEFDN_LOG_VERBOSE(
                "msg:No IBV requests are dequeued."
            );
        }
        
        return true;
    }
    
    mefdn::atomic<bool>    finished_;
};

} // namespace meult
} // namespace menps


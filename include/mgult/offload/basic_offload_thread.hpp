
#pragma once

#include <mgult/common.hpp>
#include <mgbase/atomic.hpp>
#include <mgbase/crtp_base.hpp>
#include <mgbase/profiling/clock.hpp>

namespace mgult {

template <typename Policy>
class basic_offload_thread
{
    MGBASE_POLICY_BASED_CRTP(Policy)
    
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
        
        mgbase::size_t n = 0;
        
        // TODO: exception safety
        
        MGBASE_RANGE_BASED_FOR(auto&& cmd, t)
        {
            if (!self.try_execute(
                mgbase::forward<decltype(cmd)>(cmd)
            )) {
                break;
            }

            ++n;
        }
        
        t.commit(n);
        
        if (n > 0) {
            MGBASE_LOG_DEBUG(
                "msg:Dequeued IBV requests.\t"
                "n:{}"
            ,   n
            );
        }
        else {
            MGBASE_LOG_VERBOSE(
                "msg:No IBV requests are dequeued."
            );
        }
        
        return true;
    }
    
    mgbase::atomic<bool>    finished_;
};

} // namespace mgult


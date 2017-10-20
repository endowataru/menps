
#pragma once

#include "command_queue.hpp"
#include "common/command/queue_delegator.hpp"
#include "device/ibv/rma/requester_base.hpp"

namespace menps {
namespace mecom {
namespace ibv {

class command_producer
    : protected virtual command_queue
{
    typedef command_queue   base;
    
protected:
    typedef ibv::command_code   command_code_type;
    
    command_producer() = default;
    
public:
    template <typename Params, typename Func>
    MEFDN_NODISCARD
    bool try_enqueue(
        const process_id_t      proc
    ,   const command_code_type code
    ,   Func&&                  func
    ) {
        #ifdef MECOM_IBV_ENABLE_SLEEP_QP
        auto t = base::try_enqueue(1, true);
        #else
        auto t = base::try_enqueue(1);
        #endif
        
        if (t.valid())
        {
            auto& dest = *t.begin();
            
            dest.code = code;
            dest.proc = proc;
            
            MEFDN_STATIC_ASSERT(sizeof(Params) <= command::params_size);
            mefdn::forward<Func>(func)(reinterpret_cast<Params*>(dest.arg));
            
            t.commit(1);
            
            #ifdef MECOM_IBV_ENABLE_SLEEP_QP
            if (t.is_sleeping()) { // (old_tail & 1) == 1
                MEFDN_LOG_DEBUG(
                    "msg:Awake command consumer."
                );
                
                this->notify();
            }
            #endif
            
            return true;
        }
        else
            return false;
    }
};

} // namespace ibv
} // namespace mecom
} // namespace menps


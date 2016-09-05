
#pragma once

#include "command_queue.hpp"
#include "common/command/queue_delegator.hpp"
#include "device/ibv/rma/requester_base.hpp"

namespace mgcom {
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
    MGBASE_WARN_UNUSED_RESULT
    bool try_enqueue(
        const process_id_t      proc
    ,   const command_code_type code
    ,   Func&&                  func
    ) {
        auto t = base::try_enqueue(1);
        
        if (t.valid())
        {
            auto& dest = *t.begin();
            
            dest.code = code;
            dest.proc = proc;
            
            MGBASE_STATIC_ASSERT(sizeof(Params) <= command::params_size);
            mgbase::forward<Func>(func)(reinterpret_cast<Params*>(dest.arg));
            
            t.commit(1);
            
            return true;
        }
        else
            return false;
    }
};

} // namespace ibv
} // namespace mgcom


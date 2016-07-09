
#pragma once

#include "command_queue.hpp"
#include "common/command/queue_delegator.hpp"
#include "device/ibv/rma/requester_base.hpp"

namespace mgcom {
namespace ibv {

class command_producer
    : protected virtual command_queue
{
protected:
    typedef ibv::command_code   command_code_type;
    
    command_producer() { }
    
private:
    template <typename Params, typename Func>
    struct enqueue_closure
    {
        process_id_t    proc;
        command_code    code;
        const Func&     func;
        
        void operator() (command* const dest)
        {
            dest->code = code;
            dest->proc = proc;
            
            MGBASE_STATIC_ASSERT(sizeof(Params) <= command::params_size);
            func(reinterpret_cast<Params*>(dest->arg));
        }
    };
        
public:
    template <typename Params, typename Func>
    MGBASE_WARN_UNUSED_RESULT
    bool try_enqueue(
        const process_id_t      proc
    ,   const command_code_type code
    ,   Func&&                  func
    ) {
        const bool ret = this->try_push_with_functor(
            enqueue_closure<Params, Func>{ proc, code, func }
        );
        
        return ret;
    }
};

} // namespace ibv
} // namespace mgcom


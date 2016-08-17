
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
    
    #if 0
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
    #endif
    
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
            
            t.commit();
            
            return true;
        }
        else
            return false;
        
        #if 0
        const bool ret = this->try_push_with_functor(
            enqueue_closure<Params, Func>{ proc, code, func }
        );
        
        return ret;
        #endif
    }
};

} // namespace ibv
} // namespace mgcom


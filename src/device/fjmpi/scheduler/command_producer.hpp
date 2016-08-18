
#pragma once

#include "command_queue.hpp"
#include "common/command/queue_delegator.hpp"

namespace mgcom {
namespace fjmpi {

class command_producer
    : protected virtual command_queue
{
    typedef command_queue   base;
    
public:
    command_producer()
        : del_{*this} { }
    
    template <typename Params, typename Func>
    MGBASE_WARN_UNUSED_RESULT
    bool try_enqueue(const command_code code, Func&& func)
    {
        auto t = base::try_enqueue(1);
        
        if (t.valid())
        {
            auto& dest = *t.begin();
            
            dest.code = code;
            
            MGBASE_STATIC_ASSERT(sizeof(Params) <= command::params_size);
            mgbase::forward<Func>(func)(reinterpret_cast<Params*>(dest.arg));
            
            t.commit();
            
            return true;
        }
        else
            return false;
        
        #if 0
        struct closure
        {
            command_code    code;
            const Func&     func;
            
            void operator() (command* const dest)
            {
                dest->code = code;
                
                MGBASE_STATIC_ASSERT(sizeof(Params) <= command::params_size);
                func(reinterpret_cast<Params*>(dest->arg));
            }
        };
        
        return this->try_push_with_functor(
            closure{ code, func }
        );
        #endif
    }
    
    delegator& get_delegator() MGBASE_NOEXCEPT {
        return del_;
    }
    
private:
    queue_delegator<command_queue> del_;
};

} // namespace fjmpi
} // namespace mgcom


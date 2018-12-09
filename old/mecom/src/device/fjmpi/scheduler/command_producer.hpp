
#pragma once

#include "command_queue.hpp"
#include "common/command/queue_delegator.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {

class command_producer
    : protected virtual command_queue
{
    typedef command_queue   base;
    
public:
    command_producer()
        : del_{*this} { }
    
    template <typename Params, typename Func>
    MEFDN_NODISCARD
    bool try_enqueue(const command_code code, Func&& func)
    {
        auto t = base::try_enqueue(1);
        
        if (t.valid())
        {
            auto& dest = *t.begin();
            
            dest.code = code;
            
            MEFDN_STATIC_ASSERT(sizeof(Params) <= command::params_size);
            mefdn::forward<Func>(func)(reinterpret_cast<Params*>(dest.arg));
            
            t.commit(1);
            
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
                
                MEFDN_STATIC_ASSERT(sizeof(Params) <= command::params_size);
                func(reinterpret_cast<Params*>(dest->arg));
            }
        };
        
        return this->try_push_with_functor(
            closure{ code, func }
        );
        #endif
    }
    
    delegator& get_delegator() noexcept {
        return del_;
    }
    
private:
    queue_delegator<command_queue> del_;
};

} // namespace fjmpi
} // namespace mecom
} // namespace menps


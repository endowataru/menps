
#pragma once

#include "delegator.hpp"

namespace mgcom {

template <typename Queue>
class queue_delegator
    : public delegator
{
public:
    explicit queue_delegator(Queue& q)
        : queue_(q) { }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_execute(const delegator::execute_params& params) MGBASE_OVERRIDE
    {
        auto t = queue_.try_enqueue(1);
        
        if (t.valid())
        {
            auto& dest = *t.begin();
            
            dest.set_delegated(params);
            
            t.commit();
            
            return true;
        }
        else
            return false;
    }
    
    virtual mgbase::size_t get_buffer_size() MGBASE_OVERRIDE
    {
        return sizeof(queue_.dequeue());
    }

private:
    Queue& queue_;
};

} // namespace mgcom


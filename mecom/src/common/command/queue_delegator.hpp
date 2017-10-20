
#pragma once

#include "delegator.hpp"

namespace menps {
namespace mecom {

template <typename Queue>
class queue_delegator
    : public delegator
{
public:
    explicit queue_delegator(Queue& q)
        : queue_(q) { }
    
    MEFDN_NODISCARD
    virtual bool try_execute(const delegator::execute_params& params) MEFDN_OVERRIDE
    {
        auto t = queue_.try_enqueue(1);
        
        if (t.valid())
        {
            auto& dest = *t.begin();
            
            dest.set_delegated(params);
            
            t.commit(1);
            
            return true;
        }
        else
            return false;
    }
    
    virtual mefdn::size_t get_buffer_size() MEFDN_OVERRIDE
    {
        return sizeof(queue_.dequeue());
    }

private:
    Queue& queue_;
};

} // namespace mecom
} // namespace menps


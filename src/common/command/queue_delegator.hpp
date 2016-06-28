
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
        return queue_.try_push_with_functor(closure{params});
    }
    
    virtual mgbase::size_t get_buffer_size() MGBASE_OVERRIDE
    {
        return sizeof(*queue_.peek());
    }

private:
    struct closure
    {
        explicit closure(const delegator::execute_params& p)
            : params(p) { }
        
        template <typename T>
        void operator () (T* const dest) const
        {
            dest->set_delegated(params); // Use ADL
        }
        
        const delegator::execute_params& params;
    };
    
    Queue& queue_;
};

} // namespace mgcom


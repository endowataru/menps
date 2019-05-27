
#pragma once

#include <menps/mefdn/atomic.hpp>
#include "this_thread.hpp"

namespace menps {
namespace mefdn {

// TODO: Check whether the current usages are appropriate

template <typename T>
class synchronic
{
    typedef mefdn::atomic<T>   atomic_type;
    
public:
    synchronic() = default;
    synchronic(const synchronic&) = delete;
    
    synchronic& operator = (const synchronic&) = delete;
    
    void notify(atomic_type& obj, T value)
    {
        obj.store(value, mefdn::memory_order_release);
    }
    
    void expect(const atomic_type& obj, T desired)
    {
        while (obj.load(mefdn::memory_order_acquire) != desired) {
            this_thread::yield();
        }
    }
};

} // namespace mefdn
} // namespace menps


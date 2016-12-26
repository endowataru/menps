
#pragma once

#include <mgbase/atomic.hpp>
#include "this_thread.hpp"

namespace mgbase {

template <typename T>
class synchronic
{
    typedef mgbase::atomic<T>   atomic_type;
    
public:
    synchronic() = default;
    synchronic(const synchronic&) = delete;
    
    synchronic& operator = (const synchronic&) = delete;
    
    void notify(atomic_type& obj, T value)
    {
        obj.store(value, mgbase::memory_order_release);
    }
    
    void expect(const atomic_type& obj, T desired)
    {
        while (obj.load(mgbase::memory_order_acquire) != desired) {
            this_thread::yield();
        }
    }
};

} // namespace mgbase


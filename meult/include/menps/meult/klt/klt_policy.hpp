
#pragma once

#include <menps/meult/klt/klt.hpp>

namespace menps {
namespace meult {

struct klt_policy
{
    typedef meult::klt::mutex               mutex;
    typedef meult::klt::condition_variable  condition_variable;
    
    typedef meult::klt::unique_lock<meult::klt::mutex>  unique_mutex_lock;
    
    struct this_thread
    {
        static void yield() {
            meult::klt::this_thread::yield();
        }
        
        static void detach() {
            meult::klt::this_thread::detach();
        }
    };
    
    typedef meult::klt::spinlock            spinlock;
    
    typedef meult::klt::thread              thread;
    
    template <typename Policy>
    struct thread_specific_ {
        typedef typename meult::klt::thread_specific<Policy>    type;
    };
};

namespace klt {

typedef klt_policy  ult_policy;

} // namespace klt

} // namespace meult
} // namespace menps


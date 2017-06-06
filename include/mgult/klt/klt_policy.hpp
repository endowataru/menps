
#pragma once

#include <mgult/klt/klt.hpp>

namespace mgult {

struct klt_policy
{
    typedef mgult::klt::mutex               mutex;
    typedef mgult::klt::condition_variable  condition_variable;
    
    typedef mgult::klt::unique_lock<mgult::klt::mutex>  unique_mutex_lock;
    
    struct this_thread
    {
        static void yield() {
            mgult::klt::this_thread::yield();
        }
        
        static void detach() {
            mgult::klt::this_thread::detach();
        }
    };
    
    typedef mgult::klt::spinlock            spinlock;
    
    typedef mgult::klt::thread              thread;
};

namespace klt {

typedef klt_policy  ult_policy;

} // namespace klt

} // namespace mgult


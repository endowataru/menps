
#pragma once

#include "mth_interface.hpp"

namespace mgult {
namespace backend {
namespace mth {

struct ult_policy
{
    typedef mgult::backend::mth::mutex              mutex;
    typedef mgult::backend::mth::condition_variable condition_variable;
    
    typedef mgult::backend::mth::unique_lock<mgult::backend::mth::mutex>    unique_mutex_lock;
    
    struct this_thread
    {
        static void yield() {
            mgult::klt::this_thread::yield();
        }
        
        static void detach() {
            mgult::klt::this_thread::detach();
        }
    };
    
    static myth_thread_t fork_fast(void* (* const func)(void*), void* const arg) {
        return mgult::backend::mth::fork_fast(func, arg);
    }
};

} // namespace mth
} // namespace backend
} // namespace mgult


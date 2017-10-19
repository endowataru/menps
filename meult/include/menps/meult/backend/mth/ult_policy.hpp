
#pragma once

#include "mth_interface.hpp"

namespace menps {
namespace meult {
namespace backend {
namespace mth {

struct ult_policy
{
    typedef meult::backend::mth::mutex              mutex;
    typedef meult::backend::mth::condition_variable condition_variable;
    
    typedef meult::backend::mth::unique_lock<meult::backend::mth::mutex>    unique_mutex_lock;
    
    struct this_thread
    {
        static void yield() {
            meult::klt::this_thread::yield();
        }
        
        static void detach() {
            meult::klt::this_thread::detach();
        }
    };
    
    static myth_thread_t fork_fast(void* (* const func)(void*), void* const arg) {
        return meult::backend::mth::fork_fast(func, arg);
    }
    
    typedef mefdn::spinlock            spinlock;
    
    typedef meult::backend::mth::thread thread;
    
    template <typename Policy>
    struct thread_specific_ {
        typedef typename meult::backend::mth::thread_specific<Policy>   type;
    };
    
    typedef meult::backend::mth::uncond_variable    uncond_variable;
    
    template <typename T>
    struct async_channel_ {
        typedef async_channel<T>    type;
    };
};

} // namespace mth
} // namespace backend
} // namespace meult
} // namespace menps


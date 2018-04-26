
#pragma once

#include "mth_interface.hpp"
#include "barrier.hpp"

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
    
    // Note: The type "P" is used to distinguish thread-local storages with the same type.
    template <typename P>
    using thread_specific =
        typename meult::backend::mth::thread_specific<P>;
    
    typedef meult::backend::mth::uncond_variable    uncond_variable;
    
    template <typename T>
    struct async_channel_ {
        typedef async_channel<T>    type;
    };
    
    using barrier = mth::barrier;
};

} // namespace mth
} // namespace backend
} // namespace meult
} // namespace menps


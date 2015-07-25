
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

#if (__cplusplus >= 201103L)

    using adopt_lock_t = std::adopt_lock_t;
    using lock_guard   = std::lock_guard;

#else

    struct adopt_lock_t { };
    #define adopt_lock  adopt_lock_t()
    

    template <typename Mutex>
    class lock_guard
        : noncopyable
    {
    public:
        explicit lock_guard(Mutex& mutex)
            : mutex_(mutex)
        {
            mutex.lock();
        }
        lock_guard(Mutex& mutex, adopt_lock_t) MGBASE_NOEXCEPT
            : mutex_(mutex) { }
        
        ~lock_guard() {
            mutex_.unlock();
        }

    private:
        Mutex& mutex_;
    };

#endif

}


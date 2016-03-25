
#pragma once

#include <mgbase/threading/lock_t.hpp>

namespace mgbase {

#ifdef MGBASE_CPP11_SUPPORTED

    using std::lock_guard;

#else

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

} // namespace mgbase


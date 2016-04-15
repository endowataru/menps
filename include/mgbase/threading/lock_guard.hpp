
#pragma once

#include <mgbase/threading/lock_t.hpp>

#ifdef MGBASE_CXX11_SUPPORTED

#include <mutex>

namespace mgbase {

using std::lock_guard;
}

#else // MGBASE_CXX11_SUPPORTED

namespace mgbase {

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

} // namespace mgbase

#endif // MGBASE_CXX11_SUPPORTED


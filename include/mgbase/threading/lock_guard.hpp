
#pragma once

#include <mgbase/lang.hpp>

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
    ~lock_guard() {
        mutex_.unlock();
    }

private:
    Mutex& mutex_;
};

}


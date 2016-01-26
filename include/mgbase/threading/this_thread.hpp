
#pragma once

#include <mgbase/threading/thread_id.hpp>

namespace mgbase {

namespace this_thread {

inline thread_id get_id()
{
    const pthread_t th = pthread_self();
    return thread_id(th);
}

} // namespace this_thread

} // namespace mgbase


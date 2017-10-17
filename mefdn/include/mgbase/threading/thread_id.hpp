
#pragma once

#include <mgbase/force_integer_cast.hpp>
#include <pthread.h>

namespace mgbase {

class thread_id
{
public:
    thread_id() MGBASE_NOEXCEPT
        : th_{} // TODO: Initializes to zero; is this correct in Pthreads?
        { }
    
    thread_id(pthread_t th) MGBASE_NOEXCEPT
        : th_(th) { }
    
    bool operator == (const thread_id& id) MGBASE_NOEXCEPT {
        return th_ == id.th_;
    }
    bool operator != (const thread_id& id) MGBASE_NOEXCEPT {
        return th_ != id.th_;
    }
    
    mgbase::uintptr_t to_integer() MGBASE_NOEXCEPT {
        // If pthread_t is a pointer type, then use reinterpret_cast.
        // If it is an integer type, use static_cast.
        return force_integer_cast<mgbase::uintptr_t>(th_);
    }
    
private:
    pthread_t th_;
};

} // namespace mgbase


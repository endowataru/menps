
#pragma once

#include <mgbase/lang.hpp>
#include <pthread.h>

namespace mgbase {

class thread_id
{
public:
    thread_id(pthread_t th) MGBASE_NOEXCEPT
        : th_(th) { }
    
    mgbase::uintptr_t to_integer() MGBASE_NOEXCEPT {
        return to_integer(th_);
    }
    
private:
    // If pthread_t is a pointer type, then use reinterpret_cast.
    // If it is an integer type, use static_cast.
    static mgbase::uintptr_t to_integer(void* ptr) MGBASE_NOEXCEPT {
        return reinterpret_cast<mgbase::uintptr_t>(ptr);
    }
    static mgbase::uintptr_t to_integer(mgbase::uintptr_t val) MGBASE_NOEXCEPT {
        return static_cast<mgbase::uintptr_t>(val);
    }
    
    pthread_t th_;
};

} // namespace mgbase


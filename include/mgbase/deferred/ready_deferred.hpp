
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T>
class ready_deferred
{
public:
    ready_deferred() MGBASE_EMPTY_DEFINITION
    
    /*implicit*/ ready_deferred(const T& val)
        : val_(val) { }
    
    T& get() {
        return val_;
    }
    
    const T& get() const MGBASE_NOEXCEPT {
        return val_;
    }
    
private:
    T val_;
};

template <>
class ready_deferred<void>
{
public:
    void get() { }
};

namespace /*unnamed*/ {

inline MGBASE_ALWAYS_INLINE ready_deferred<void> make_ready_deferred() MGBASE_NOEXCEPT {
    return ready_deferred<void>();
}

template <typename T>
inline MGBASE_ALWAYS_INLINE ready_deferred<T> make_ready_deferred(const T& val) {
    return ready_deferred<T>(val);
}

} // unnamed namespace

} // namespace mgbase


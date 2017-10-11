
#pragma once

#include <mgbase/utility/move.hpp>
#include <mgbase/utility/forward.hpp>
#include <mgbase/assert.hpp>

namespace mgult {

template <typename T>
class async_status
{
public:
    async_status(const async_status&) /*may throw*/ = default;
    async_status& operator = (const async_status&) /*may throw*/ = default;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_2(async_status, ready_, val_)
    
    bool is_ready() const {
        return this->ready_;
    }
    
    T& get() {
        MGBASE_ASSERT(this->ready_);
        return this->val_;
    }
    
    template <typename U>
    static async_status make_ready(U&& val) {
        return { mgbase::forward<U>(val) };
    }
    static async_status make_deferred() {
        return { };
    }
    
private:
    async_status() MGBASE_NOEXCEPT
        : ready_{false}
        // val_ is uninitialized
    { }
    
    async_status(const T& val)
        : ready_{true}
        , val_(val)
    { }
    async_status(T&& val)
        : ready_{true}
        , val_(mgbase::move(val))
    { }
    
    bool    ready_;
    T       val_;
};

template <>
class async_status<void>
{
public:
    async_status(const async_status&) MGBASE_DEFAULT_NOEXCEPT = default;
    async_status& operator = (const async_status&) MGBASE_DEFAULT_NOEXCEPT = default;
    
    bool is_ready() const MGBASE_NOEXCEPT {
        return this->ready_;
    }
    void get() const MGBASE_NOEXCEPT {  }
    
    static async_status make_ready() MGBASE_NOEXCEPT {
        return { true };
    }
    static async_status make_deferred() MGBASE_NOEXCEPT {
        return { false };
    }
    
private:
    async_status(const bool ready) MGBASE_NOEXCEPT
        : ready_{ready}
    { }
    
    bool    ready_;
};

template <typename T>
inline async_status<T> make_async_ready(T&& val) {
    return async_status<T>::make_ready(mgbase::forward<T>(val));
}
inline async_status<void> make_async_ready() {
    return async_status<void>::make_ready();
}

template <typename T>
inline async_status<T> make_async_deferred() {
    return async_status<T>::make_deferred();
}

// For old GCC, we cannot put these functions
// as member functions in async_status<t>.
template <typename T>
inline T& async_get(async_status<T>& s) {
    return s.get();
}
template <typename T>
inline T&& async_get(async_status<T>&& s) {
    return mgbase::move(s.get());
}
inline void async_get(const async_status<void>& s) {
    return s.get();
}

} // namespace mgult


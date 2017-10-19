
#pragma once

#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace meult {

template <typename T>
class async_status
{
public:
    async_status(const async_status&) /*may throw*/ = default;
    async_status& operator = (const async_status&) /*may throw*/ = default;
    
    bool is_ready() const {
        return this->ready_;
    }
    
    T& get() {
        MEFDN_ASSERT(this->ready_);
        return this->val_;
    }
    
    template <typename U>
    static async_status make_ready(U&& val) {
        return { mefdn::forward<U>(val) };
    }
    static async_status make_deferred() {
        return { };
    }
    
private:
    async_status() noexcept
        : ready_{false}
        // val_ is uninitialized
    { }
    
    async_status(const T& val)
        : ready_{true}
        , val_(val)
    { }
    async_status(T&& val)
        : ready_{true}
        , val_(mefdn::move(val))
    { }
    
    bool    ready_;
    T       val_;
};

template <>
class async_status<void>
{
public:
    async_status(const async_status&) noexcept = default;
    async_status& operator = (const async_status&) noexcept = default;
    
    bool is_ready() const noexcept {
        return this->ready_;
    }
    void get() const noexcept {  }
    
    static async_status make_ready() noexcept {
        return { true };
    }
    static async_status make_deferred() noexcept {
        return { false };
    }
    
private:
    async_status(const bool ready) noexcept
        : ready_{ready}
    { }
    
    bool    ready_;
};

template <typename T>
inline async_status<T> make_async_ready(T&& val) {
    return async_status<T>::make_ready(mefdn::forward<T>(val));
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
    return mefdn::move(s.get());
}
inline void async_get(const async_status<void>& s) {
    return s.get();
}

} // namespace meult
} // namespace menps


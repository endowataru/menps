
#pragma once

#include <mgbase/lang.hpp>

namespace mgult {

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
        MGBASE_ASSERT(this->ready_);
        return this->val_;
    }
    
    static async_status make_ready(const T& val) {
        return { val };
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
inline async_status<T> make_async_ready(const T& val) {
    return async_status<T>::make_ready(val);
}
inline async_status<void> make_async_ready() {
    return async_status<void>::make_ready();
}

template <typename T>
inline async_status<T> make_async_deferred() {
    return async_status<T>::make_deferred();
}

} // namespace mgult


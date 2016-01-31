
#pragma once

#include "continuation.hpp"
#include "ready_deferred.hpp"
#include <mgbase/assert.hpp>
#include <mgbase/function_traits.hpp>

namespace mgbase {

namespace detail {

template <typename Derived, typename T>
class deferred_base
{
public:
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT resumable set_terminal(T* dest);
    
    T wait() {
        T result;
        resumable res = this->set_terminal(&result);
        while (!res.checked_resume()) { }
        return result;
    }
    
private:
    Derived& derived() MGBASE_NOEXCEPT { return *static_cast<Derived*>(this); }
    const Derived& derived() const MGBASE_NOEXCEPT { return *static_cast<const Derived*>(this); }
};

template <typename Derived>
class deferred_base<Derived, void>
{
public:
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT resumable set_terminal();
    
    void wait() {
        resumable res = this->set_terminal();
        while (!res.checked_resume()) { }
    }
    
private:
    Derived& derived() MGBASE_NOEXCEPT { return *static_cast<Derived*>(this); }
    const Derived& derived() const MGBASE_NOEXCEPT { return *static_cast<const Derived*>(this); }
};

} // namespace detail

template <typename T>
class deferred
    : public detail::deferred_base<deferred<T>, T>
    , private value_wrapper<T>
{
    /*
     * The value is defined as a base class (not a member)
     * in order to utilize the empty base optimization
     * when T is void.
     */
    typedef value_wrapper<T>   base_value;
    
public:
    /*implicit*/ deferred(const value_wrapper<T>& val)
        : base_value(val)
        , cont_(MGBASE_NULLPTR)
        { }
    
    // For unwrapping
    /*implicit*/ deferred(const value_wrapper< deferred<T> >& df) {
        *this = df.get();
    }
    
    explicit deferred(continuation<T>& cont, const resumable& res) MGBASE_NOEXCEPT
        : res_(res)
        , cont_(&cont)
        { }
    
    #ifdef MGBASE_CPP11_SUPPORTED
    deferred(const deferred&) = default;
    
    deferred& operator = (const deferred&) = default;
    #endif
    
    continuation<T>* get_continuation() const MGBASE_NOEXCEPT {
        return cont_;
    }
    
    void set_continuation(const typename continuation<T>::function_type& func)
    {
        MGBASE_ASSERT(cont_ != MGBASE_NULLPTR);
        cont_->set(func);
    }
    
    resumable& get_resumable() MGBASE_NOEXCEPT {
        return res_;
    }
    
    bool resume() {
        return res_.resume();
    }
    
    value_wrapper<T>& to_ready() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(cont_ == MGBASE_NULLPTR);
        return *this;
    }
    const value_wrapper<T>& to_ready() const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(cont_ == MGBASE_NULLPTR);
        return *this;
    }

private:
    resumable        res_;
    continuation<T>* cont_;
};

namespace detail {

template <typename T>
struct remove_deferred {
    typedef T   type;
};

template <typename T>
struct remove_deferred< deferred<T> > {
    typedef T   type;
};
template <typename T>
struct remove_deferred< value_wrapper<T> > {
    typedef T   type;
};

template <typename Signature>
struct deferred_result {
    typedef typename remove_deferred<
        typename mgbase::function_traits<Signature>::result_type
    >::type
    type;
};

} // namespace detail


} // namespace mgbase


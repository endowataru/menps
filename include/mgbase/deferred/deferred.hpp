
#pragma once

#include "continuation.hpp"
#include "ready_deferred.hpp"
#include <mgbase/assert.hpp>
#include <mgbase/function_traits.hpp>
#include <mgbase/move.hpp>

namespace mgbase {

template <typename T>
class deferred;

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


template <typename Derived, typename T>
class deferred_base
{
public:
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    resumable set_terminal(T* dest);
    
    T wait() {
        T result;
        resumable res = this->set_terminal(&result);
        while (!res.checked_resume()) { }
        return result;
    }
    
    template <typename F>
    T wait(F f) {
        T result;
        resumable res = this->set_terminal(&result);
        while (!res.checked_resume()) { f(); }
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
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    resumable set_terminal();
    
    void wait() {
        resumable res = this->set_terminal();
        while (!res.checked_resume()) { }
    }
    
    template <typename F>
    void wait(F f) {
        resumable res = this->set_terminal();
        while (!res.checked_resume()) { f(); }
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
     * when T is void or an empty class.
     */
    typedef value_wrapper<T>   base_value;
    
public:
    MGBASE_CONSTEXPR /*implicit*/ deferred(const value_wrapper<T>& val)
        : base_value(val)
        , cont_(MGBASE_NULLPTR)
        { }
    
    #ifdef MGBASE_CXX11_SUPPORTED
    MGBASE_CONSTEXPR /*implicit*/ deferred(value_wrapper<T>&& val)
        : base_value(mgbase::move(val))
        , cont_(MGBASE_NULLPTR)
        { }
    #endif
    
    // For unwrapping
    MGBASE_CONSTEXPR_CXX14 /*implicit*/ deferred(const value_wrapper< deferred<T> >& df) {
        *this = df.get();
    }
    
    #ifdef MGBASE_CXX11_SUPPORTED
    MGBASE_CONSTEXPR_CXX14 /*implicit*/ deferred(value_wrapper< deferred<T> >&& df) {
        *this = mgbase::move(df.get());
    }
    #endif
    
    deferred(continuation<T>& cont, const resumable& res) MGBASE_NOEXCEPT
        : res_(res)
        , cont_(&cont)
        { }
    
    #ifdef MGBASE_CXX11_SUPPORTED
    // Move-only type
    MGBASE_CONSTEXPR deferred(const deferred&) MGBASE_NOEXCEPT = delete;
    MGBASE_CONSTEXPR deferred(deferred&&) MGBASE_NOEXCEPT = default;
    
    // Move-only type
    deferred& operator = (const deferred&) MGBASE_NOEXCEPT = delete;
    deferred& operator = (deferred&&) MGBASE_NOEXCEPT = default;
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
    
    template <typename Signature, Signature Func, typename CB>
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    deferred<typename detail::deferred_result<Signature>::type>
    add_continuation(inlined_function<Signature, Func>, CB&)
    MGBASE_IF_CXX11_SUPPORTED(&&);

private:
    resumable        res_;
    continuation<T>* cont_;
};

} // namespace mgbase


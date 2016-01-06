
#pragma once

#include "continuation.hpp"
#include <mgbase/assert.hpp>

namespace mgbase {

template <typename T>
class deferred
    :  private ready_deferred<T>
{
    /*
     * The value is defined as a base class (not a member)
     * in order to utilize the empty base optimization
     * when T is void.
     */
    typedef ready_deferred<T>   base_value;
    
public:
    /*implicit*/ deferred(const ready_deferred<T>& val)
        : cont_(MGBASE_NULLPTR)
        , base_value(val)
        { }
    
    explicit deferred(continuation<T>& cont, const resumable& res) MGBASE_NOEXCEPT
        : cont_(&cont)
        , res_(res)
        { }
    
    #ifdef MGBASE_CPP11_SUPPORTED
    deferred(const deferred&) = default;
    
    deferred& operator = (const deferred&) = default;
    #endif
    
    continuation<T>* get_continuation() const MGBASE_NOEXCEPT {
        return cont_;
    }
    
    resumable& get_resumable() MGBASE_NOEXCEPT {
        return res_;
    }
    
    void resume() {
        res_.resume();
    }
    
    ready_deferred<T>& to_ready() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(cont_ == MGBASE_NULLPTR);
        return *this;
    }
    const ready_deferred<T>& to_ready() const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(cont_ == MGBASE_NULLPTR);
        return *this;
    }
    

private:
    resumable        res_;
    continuation<T>* cont_;
};

namespace detail {

MGBASE_IF_CPP11_SUPPORTED(namespace /*unnamed*/ {, )

template <typename Signature, Signature* Func, typename T, typename U>
struct add_continuation_handler
{
    static MGBASE_ALWAYS_INLINE resumable transfer(continuation<U>& next_cont, const ready_deferred<T>& val)
    {
        return next_cont.call(
            call_and_make_ready<U>(Func, val)
        );
    }
};

MGBASE_IF_CPP11_SUPPORTED(} /* unnamed namespace */,)

} // namespace detail

namespace /*unnamed*/ {

template <typename Signature, Signature* Func, typename T, typename U>
inline MGBASE_ALWAYS_INLINE deferred<U> add_continuation(continuation<U>& next_cont, deferred<T> df)
{
    continuation<T>* const current_cont = df.get_continuation();
    
    if (MGBASE_LIKELY(current_cont == MGBASE_NULLPTR))
    {
        return call_and_make_ready<U>(Func, df.to_ready());
    }
    else
    {
        current_cont->set(
            make_binded_function<
                resumable (continuation<U>&, const ready_deferred<T>&)
            ,   &detail::add_continuation_handler<Signature, Func, T, U>::transfer
            >
            (&next_cont)
        );
        
        return deferred<U>(next_cont, df.get_resumable());
    }
}

} // unnamed namespace

namespace detail {

MGBASE_IF_CPP11_SUPPORTED(namespace /*unnamed*/ {, )

template <typename T, typename CB, continuation<T> CB::*Cont, deferred<T> (*Resumed)(CB&)>
struct make_deferred_handler
{
    static MGBASE_ALWAYS_INLINE resumable transfer(CB& cb)
    {
        deferred<T> df = Resumed(cb);
        
        continuation<T>* const current_cont = df.get_continuation();
        
        if (MGBASE_LIKELY(current_cont == MGBASE_NULLPTR))
        {
            continuation<T>& next_cont = cb.*Cont;
            return next_cont.call(df.to_ready());
        }
        else {
            MGBASE_ASSERT(current_cont == &(cb.*Cont));
            return df.get_resumable();
        }
    }
};

MGBASE_IF_CPP11_SUPPORTED(} /* unnamed namespace */,)

} // namespace detail

namespace /*unnamed*/ {

template <typename T, typename CB, continuation<T> CB::*Cont, deferred<T> (*Resumed)(CB&)>
inline deferred<T> make_deferred(CB& cb)
{
    return deferred<T>(
        cb.*Cont
    ,   make_binded_function<
            resumable (CB&)
        ,   detail::make_deferred_handler<T, CB, Cont, Resumed>::transfer
        >
        (&cb)
    );
}

} // unnamed namespace


} // namespace mgbase


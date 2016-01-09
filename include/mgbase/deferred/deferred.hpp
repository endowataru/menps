
#pragma once

#include "continuation.hpp"
#include <mgbase/assert.hpp>
#include <mgbase/function_traits.hpp>

namespace mgbase {

template <typename T>
class deferred
    : private ready_deferred<T>
{
    /*
     * The value is defined as a base class (not a member)
     * in order to utilize the empty base optimization
     * when T is void.
     */
    typedef ready_deferred<T>   base_value;
    
public:
    /*implicit*/ deferred(const ready_deferred<T>& val)
        : base_value(val)
        , cont_(MGBASE_NULLPTR)
        { }
    
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
    
    inline T wait();

private:
    resumable        res_;
    continuation<T>* cont_;
};


namespace detail {

#ifdef MGBASE_IF_CPP11_SUPPORTED
namespace /*unnamed*/ {
#endif

template <typename T>
struct deferred_wait_handler
{
    struct argument {
        bool finished;
        ready_deferred<T> result;
    };
    
    static resumable func(argument& arg, const ready_deferred<T>& df) MGBASE_NOEXCEPT {
        arg.result = df;
        arg.finished = true;
        return make_empty_resumable();
    }
};

#ifdef MGBASE_IF_CPP11_SUPPORTED
} // unnamed namespace
#endif

} // namespace detail

template <typename T>
T deferred<T>::wait()
{
    continuation<T>* const current_cont = this->get_continuation();
    
    if (MGBASE_LIKELY(current_cont == MGBASE_NULLPTR))
    {
        return this->to_ready().get();
    }
    else
    {
        typedef detail::deferred_wait_handler<T>    handler;
        typedef typename handler::argument          argument;
        
        argument arg;
        arg.finished = false;
        
        this->set_continuation(
            make_binded_function<
                resumable (argument&, const ready_deferred<T>&)
            ,   &handler::func
            >
            (&arg)
        );
        
        while (!arg.finished)
        {
            this->resume();
        }
        
        return arg.result.get();
    }
}


template <typename T>
struct remove_deferred {
    typedef T   type;
};

template <typename T>
struct remove_deferred< deferred<T> > {
    typedef T   type;
};
template <typename T>
struct remove_deferred< ready_deferred<T> > {
    typedef T   type;
};

namespace detail {

template <typename Signature>
struct deferred_result {
    typedef typename remove_deferred<
        typename mgbase::function_traits<Signature>::result_type
    >::type
    type;
};

} // namespace detail

} // namespace mgbase


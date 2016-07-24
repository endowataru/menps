
#pragma once

#include <mgbase/callback_function.hpp>
#include <mgbase/value_wrapper.hpp>

namespace mgbase {

class resumable
{
public:
    typedef callback_function<resumable ()>    function_type;
    
    #ifdef MGBASE_CXX11_SUPPORTED
    
    resumable() MGBASE_NOEXCEPT = default;
    
    resumable(const resumable&) MGBASE_NOEXCEPT = default;
    resumable(resumable&) MGBASE_NOEXCEPT = default;
    
    /*implicit*/ resumable(const function_type& func) MGBASE_NOEXCEPT
        : func_(func)
        { }
    
    resumable& operator = (const resumable&) MGBASE_NOEXCEPT = default;
    resumable& operator = (resumable&&) MGBASE_NOEXCEPT = default;
    
    #endif
    
    MGBASE_ALWAYS_INLINE
    static resumable create(const function_type& func) MGBASE_NOEXCEPT {
        resumable r;
        r.func_ = func;
        return r;
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    bool resume()
    {
        const resumable r = func_();
        func_ = r.func_;
        
        return empty();
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    bool checked_resume()
    {
        if (empty())
            return true;
        else
            return resume();
    }
    
    MGBASE_ALWAYS_INLINE
    bool empty() const MGBASE_NOEXCEPT
    {
        return func_.empty();
    }

private:
    function_type func_;
};

namespace /*unnamed*/ {

template <typename Signature, Signature Func, typename CB>
MGBASE_ALWAYS_INLINE
resumable make_resumable(
    inlined_function<Signature, Func>   func
,   CB&                                 cb
) MGBASE_NOEXCEPT
{
    return resumable::create(
        mgbase::make_callback_function(
            mgbase::bind1st_of_1(
                func
            ,   mgbase::wrap_reference(cb)
            )
        )
    );
}

MGBASE_ALWAYS_INLINE
resumable make_empty_resumable() MGBASE_NOEXCEPT {
    return resumable::create(
        callback_function<resumable ()>::create_empty()
    );
}

} // unnamed namespace

} // namespace mgbase


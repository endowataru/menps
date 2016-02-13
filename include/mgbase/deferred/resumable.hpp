
#pragma once

#include <mgbase/callback_function.hpp>
#include <mgbase/value_wrapper.hpp>

namespace mgbase {

class resumable
{
public:
    typedef callback_function<resumable ()>    function_type;
    
    #if 0
    resumable() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    /*implicit*/ resumable(const bound_function<resumable ()>& func) MGBASE_NOEXCEPT
        : func_(func)
        { }
    #endif
    
    static resumable create(const function_type& func) {
        resumable r;
        r.func_ = func;
        return r;
    }
    
    bool resume()
    {
        const resumable r = func_();
        func_ = r.func_;
        
        return empty();
    }
    
    bool checked_resume()
    {
        if (empty())
            return true;
        else
            return resume();
    }
    
    bool empty() const MGBASE_NOEXCEPT
    {
        return func_.empty();
    }

private:
    function_type func_;
};

namespace /*unnamed*/ {

template <
    typename Signature
,   Signature* Func
,   typename CB
>
inline resumable make_resumable(CB& cb)
{
    return resumable::create(
        mgbase::make_callback_function(
            mgbase::bind1st_of_1(
                MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(Func)
            ,   mgbase::wrap_reference(cb)
            )
        )
    );
}

inline resumable make_empty_resumable() MGBASE_NOEXCEPT {
    return resumable::create(
        callback_function<resumable ()>::create_empty()
    );
}

} // unnamed namespace

} // namespace mgbase


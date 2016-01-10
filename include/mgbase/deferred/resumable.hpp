
#pragma once

#include <mgbase/bound_function.hpp>

namespace mgbase {

class resumable
{
public:
    typedef bound_function<resumable ()>    function_type;
    
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

inline resumable make_resumable(resumable::function_type func) MGBASE_NOEXCEPT {
    return resumable::create(func);
}

inline resumable make_empty_resumable() MGBASE_NOEXCEPT {
    return resumable::create(
        bound_function<resumable ()>::create_empty()
    );
}

} // unnamed namespace

} // namespace mgbase


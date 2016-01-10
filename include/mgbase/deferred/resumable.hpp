
#pragma once

#include <mgbase/bound_function.hpp>

namespace mgbase {

class resumable
{
public:
    resumable() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    /*implicit*/ resumable(const bound_function<resumable ()>& func) MGBASE_NOEXCEPT
        : func_(func)
        { }
    
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
    bound_function<resumable ()> func_;
};

namespace /*unnamed*/ {

inline resumable make_empty_resumable() {
    return resumable(
        bound_function<resumable ()>::create_empty()
    );
}

} // unnamed namespace

} // namespace mgbase


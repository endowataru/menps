
#pragma once

#include <mgbase/binded_function.hpp>

namespace mgbase {

class resumable
{
public:
    resumable() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    /*implicit*/ resumable(const binded_function<resumable ()>& func) MGBASE_NOEXCEPT
        : func_(func)
        { }
    
    void resume()
    {
        resumable r = func_();
        func_ = r.func_;
    }

private:
    binded_function<resumable ()> func_;
};

} // namespace mgbase


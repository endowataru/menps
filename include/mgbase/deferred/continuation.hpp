
#pragma once

#include "continuation.h"
#include "resumable.hpp"
#include <mgbase/value_wrapper.hpp>

namespace mgbase {

template <typename T>
class continuation
{
public:
    typedef bound_function<resumable (const value_wrapper<T>&)>   function_type;
    
    #if MGBASE_CPP11_SUPPORTED
    continuation() noexcept = default;
    #endif
    
    void set(const function_type& func)
    {
        func_ = func;
    }
    
    const function_type& get() const MGBASE_NOEXCEPT {
        return func_;
    }
    
    resumable call(const value_wrapper<T>& value)
    {
        return func_(value);
    }
    
private:
    function_type func_;
};

} // namespace mgbase


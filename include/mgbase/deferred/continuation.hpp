
#pragma once

#include "continuation.h"
#include "resumable.hpp"
#include "ready_deferred.hpp"

namespace mgbase {

template <typename T>
class continuation
{
public:
    typedef binded_function<resumable (const ready_deferred<T>&)>   function_type;
    
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
    
    resumable call(const ready_deferred<T>& value)
    {
        return func_(value);
    }
    
private:
    function_type func_;
};

} // namespace mgbase


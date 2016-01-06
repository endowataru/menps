
#pragma once

#include "continuation.h"
#include "resumable.hpp"
#include "ready_deferred.hpp"

namespace mgbase {

template <typename T>
class continuation
{
public:
    continuation() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    void set(const binded_function<resumable (const ready_deferred<T>&)>& func)
    {
        func_ = func;
    }
    
    resumable call(const ready_deferred<T>& value)
    {
        return func_(value);
    }
    
private:
    binded_function<resumable (const ready_deferred<T>&)> func_;
};

} // namespace mgbase


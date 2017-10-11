
#pragma once

#include "uncond_variable.hpp"

namespace mgult {
namespace backend {
namespace mth {

template <typename T>
class async_channel
{
public:
    async_channel()
        : u_()
        // Note: val_ is not initialized
    { }
    
    template <typename U>
    void set_value(U&& val)
    {
        this->val_ = mgbase::forward<U>(val);
        
        u_.notify();
    }
    
    template <typename Func>
    T& get(Func /*func*/)
    {
        u_.wait();
        
        return this->val_;
    }
    
private:
    uncond_variable u_;
    T               val_;
};

template <>
class async_channel<void>
{
public:
    async_channel()
        : u_()
        // Note: val_ is not initialized
    { }
    
    void set_value()
    {
        u_.notify();
    }
    
    template <typename Func>
    void get(Func /*func*/)
    {
        u_.wait();
    }
    
private:
    uncond_variable u_;
};

} // namespace mth
} // namespace backend
} // namespace mgult


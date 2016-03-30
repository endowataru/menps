
#pragma once

#include "inlined_function.hpp"

namespace mgbase {

template <typename Signature>
class callback_function;

template <typename Result>
class callback_function<Result ()>
{
    typedef Result (func_type)(void*);

public:
    #ifdef MGBASE_CPP11_SUPPORTED
    callback_function() = default;
    callback_function(const callback_function&) = default;
    
    callback_function(func_type* const func, void* const data)
        : func_(func), data_(data) { }
    
    callback_function& operator = (const callback_function&) = default;
    #endif
    
    static callback_function create(func_type* const func, void* const data) MGBASE_NOEXCEPT
    {
        callback_function f;
        f.func_ = func;
        f.data_ = data;
        return f;
    }
    
    static callback_function create_empty() MGBASE_NOEXCEPT
    {
        return create(MGBASE_NULLPTR, MGBASE_NULLPTR);
    }
    
    MGBASE_ALWAYS_INLINE Result operator () () const {
        return func_(data_);
    }
    
    bool empty() const MGBASE_NOEXCEPT {
        return func_ == MGBASE_NULLPTR;
    }
    
private:
    func_type*  func_;
    void*       data_;
};

template <typename Result, typename Arg1>
class callback_function<Result (Arg1)>
{
    typedef Result (func_type)(void*, Arg1);
    
public:
    #ifdef MGBASE_CPP11_SUPPORTED
    callback_function() = default;
    callback_function(const callback_function&) = default;
    
    callback_function(func_type* const func, void* const data)
        : func_(func), data_(data) { }
    
    callback_function& operator = (const callback_function&) = default;
    #endif
    
    static callback_function create(func_type* const func, void* const data) MGBASE_NOEXCEPT
    {
        callback_function f;
        f.func_ = func;
        f.data_ = data;
        return f;
    }
    
    static callback_function create_empty() MGBASE_NOEXCEPT
    {
        return create(MGBASE_NULLPTR, MGBASE_NULLPTR);
    }
    
    MGBASE_ALWAYS_INLINE Result operator () (Arg1 arg1) const {
        return func_(data_, arg1);
    }
    
    bool empty() const MGBASE_NOEXCEPT {
        return func_ == MGBASE_NULLPTR;
    }
private:
    func_type*  func_;
    void*       data_;
};

} // namespace mgbase


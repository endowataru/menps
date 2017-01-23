
#pragma once

#include <mgbase/lang.hpp>
#include <mgbase/explicit_operator_bool.hpp>
#include <mgbase/utility/forward.hpp>
#include <mgbase/type_traits/is_trivially_copyable.hpp>
#include <mgbase/type_traits/is_trivially_destructible.hpp>
#include <mgbase/assert.hpp>
#include <array>
#include <new> // for placement new

namespace mgbase {

// Reference:
// http://codereview.stackexchange.com/questions/58447/function-wrapper-like-stdfunction-that-uses-small-buffer-allocation

template <typename Signature>
class callback;

template <typename Result, typename... Args>
class callback<Result (Args...)>
{
    typedef std::array<mgbase::int64_t, 3>  storage_type;
    typedef Result (func_type)(const storage_type&, Args...);
    
public:
    callback()
        : func_{MGBASE_NULLPTR} { }
    
    callback(const callback&) MGBASE_DEFAULT_NOEXCEPT = default;
    
    template <typename Func>
    /*implicit*/ callback(const Func& func)
        : func_(&call<Func>)
    {
        MGBASE_STATIC_ASSERT(sizeof(func_) <= sizeof(void*));
        MGBASE_STATIC_ASSERT(mgbase::is_trivially_copyable<Func>::value);
        MGBASE_STATIC_ASSERT(mgbase::is_trivially_destructible<Func>::value);
        
        new (storage_.data()) Func(func);
    }
    
    ~callback() = default;
    
    callback& operator = (const callback&) MGBASE_DEFAULT_NOEXCEPT = default;
    
    template <typename... As>
    Result operator () (As&&... args) const {
        MGBASE_ASSERT(func_ != MGBASE_NULLPTR);
        return func_(storage_, mgbase::forward<As>(args)...);
    }
    
    MGBASE_EXPLICIT_OPERATOR_BOOL()
    
    bool operator ! () const MGBASE_NOEXCEPT {
        return func_ == MGBASE_NULLPTR;
    }
    
private:
    template <typename Func>
    static Result call(const storage_type& storage, Args... args)
    {
        auto& func = reinterpret_cast<const Func&>(storage);
        return func(args...);
    }
    
    func_type*      func_;
    storage_type    storage_;
};

} // namespace mgbase


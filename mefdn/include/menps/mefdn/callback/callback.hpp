
#pragma once

#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/assert.hpp>
#include <array>
#include <new> // for placement new

namespace menps {
namespace mefdn {

// Reference:
// http://codereview.stackexchange.com/questions/58447/function-wrapper-like-stdfunction-that-uses-small-buffer-allocation

template <typename Signature>
class callback;

template <typename Result, typename... Args>
class callback<Result (Args...)>
{
    typedef std::array<mefdn::int64_t, 3>  storage_type;
    typedef Result (func_type)(const storage_type&, Args...);
    
public:
    callback()
        : func_{nullptr} { }
    
    callback(const callback&) noexcept = default;
    
    template <typename Func>
    /*implicit*/ callback(const Func& func)
        : func_(&call<Func>)
    {
        MEFDN_STATIC_ASSERT(sizeof(func_) <= sizeof(void*));
        MEFDN_STATIC_ASSERT(mefdn::is_trivially_copyable<Func>::value);
        MEFDN_STATIC_ASSERT(mefdn::is_trivially_destructible<Func>::value);
        
        new (storage_.data()) Func(func);
    }
    
    ~callback() = default;
    
    callback& operator = (const callback&) noexcept = default;
    
    template <typename... As>
    Result operator () (As&&... args) const {
        MEFDN_ASSERT(func_ != nullptr);
        return func_(storage_, mefdn::forward<As>(args)...);
    }
    
    MEFDN_EXPLICIT_OPERATOR_BOOL()
    
    bool operator ! () const noexcept {
        return func_ == nullptr;
    }
    
private:
    template <typename Func>
    static Result call(const storage_type& storage, Args... args)
    {
        auto& func = reinterpret_cast<const Func&>(storage);
        
        // Explicitly cast to the original type
        // to pass the rvalue reference.
        // TODO: This is not the fundamental solution
        //       because move-only non-reference types cannot be passed.
        return func(static_cast<Args>(args)...);
    }
    
    func_type*      func_;
    storage_type    storage_;
};

} // namespace mefdn
} // namespace menps


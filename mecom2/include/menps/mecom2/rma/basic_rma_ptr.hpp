
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/ptr_facade.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_rma_ptr
    : public mefdn::ptr_facade<P>
{
    using base = mefdn::ptr_facade<P>;
    using minfo_type = typename P::minfo_type;
    
    using size_type = typename P::size_type;
    using difference_type = typename P::difference_type;
    
public:
    using element_type = typename P::element_type;
    
    // Note: Default initializer doesn't initialize members for performance.
    basic_rma_ptr() noexcept = default;
    
    /*implicit*/ basic_rma_ptr(mefdn::nullptr_t)
        : ptr_(nullptr)
        , minfo_(nullptr)
    { }
    
    template <typename P1>
    /*implicit*/ basic_rma_ptr(const basic_rma_ptr<P1>& other)
        // Forward to the copy constructor.
        : basic_rma_ptr(
            other.template implicit_cast_to<element_type>()
        )
    { }
    
    /*implicit*/ basic_rma_ptr(
        element_type* const ptr
    ,   minfo_type* const   minfo
    ) noexcept
        : ptr_(ptr)
        , minfo_(minfo)
    { }
    
    minfo_type* get_minfo() const noexcept {
        return this->minfo_;
    }
    
    element_type* get_ptr() const noexcept {
        return this->ptr_;
    }
    
private:
    template <typename P1>
    friend class mefdn::ptr_facade;
    
    template <typename U>
    typename P::template rebind_t<U> cast_to() const noexcept {
        return typename P::template rebind_t<U>(
            reinterpret_cast<U*>(this->ptr_)
        ,   this->minfo_
        );
    }
    void advance(const difference_type diff) noexcept {
        this->ptr_ += diff;
    }
    bool is_null() const noexcept {
        return this->ptr_ == nullptr;
    }
    
    element_type*   ptr_;
    minfo_type*     minfo_;
};

} // namespace mecom2
} // namespace menps


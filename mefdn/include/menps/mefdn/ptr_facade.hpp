
#pragma once

#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace mefdn {

template <typename P>
class ptr_facade
{
    MEFDN_DEFINE_DERIVED(P)
    
    using size_type = typename P::size_type;
    using difference_type = typename P::difference_type;
    
public:
    using element_type = typename P::element_type;
    
    ptr_facade() noexcept = default;
    
    derived_type& operator += (const difference_type diff) noexcept {
        auto& self = this->derived();
        self.advance(diff);
        return self;
    }
    
    derived_type operator + (const difference_type diff) const noexcept {
        auto& self = this->derived();
        auto result = self;
        result += diff;
        return result; // RVO
    }
    
    template <typename U,
        // Delay the instantiation of this member function.
        typename T1 = element_type
    >
    typename P::template rebind_t<U> member(U (T1::* const q)) const noexcept {
        // Calculate the offset of the member q in element_type.
        const auto offset =
            mefdn::distance_in_bytes(
                nullptr
            ,   &(static_cast<element_type*>(nullptr)->*q)
            );
            
        auto& self = this->derived();
        
        auto result = self.template cast_to<mefdn::byte>();
        result.advance(offset);
        
        return result.template cast_to<U>();
    }
    
    template <typename U>
    typename P::template rebind_t<U> implicit_cast_to() const noexcept
    {
        // Dummy code for type checking.
        MEFDN_MAYBE_UNUSED
        element_type* const dummy = nullptr;
        MEFDN_MAYBE_UNUSED
        U* const dummy2 = dummy;
        
        auto& self = this->derived();
        return self.template cast_to<U>();
    }
    
    template <typename U>
    typename P::template rebind_t<U> static_cast_to() const noexcept
    {
        // Dummy code for type checking.
        MEFDN_MAYBE_UNUSED
        element_type* const dummy = nullptr;
        MEFDN_MAYBE_UNUSED
        U* const dummy2 = static_cast<U*>(dummy);
        
        auto& self = this->derived();
        return self.template cast_to<U>();
    }
    
private:
/*
    template <typename U>
    typename P::rebind_t<U> cast_to() const;
    
    void advance(difference_type);
    
    bool is_null() const;
*/
};

} // namespace mefdn
} // namespace menps


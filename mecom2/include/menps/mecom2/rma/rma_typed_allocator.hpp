
#pragma once

#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

namespace detail {

template <typename P, typename T>
struct rma_typed_allocator_helper;

template <typename P, typename T>
struct rma_typed_allocator_helper<P, T []> {
    using array_type = typename P::template unique_public_ptr<T []>;
};

} // namespace detail

template <typename P>
class rma_typed_allocator
{
    MEFDN_DEFINE_DERIVED(P)
    
    using size_type = typename P::size_type;
    
public:
    template <typename T>
    typename detail::rma_typed_allocator_helper<P, T>::array_type
    make_unique(const size_type n)
    {
        using element_type = mefdn::remove_extent_t<T>;
        
        auto& self = this->derived();
        const auto size = n * sizeof(element_type);
        const auto p = self.untyped_allocate(size);
        
        // Initialize values (to imitate the behavior of std::make_unique).
        // Note: Use () instead of {} to support GCC 4.x (buggy versions).
        new (p) element_type[n] ();
        
        return typename P::template unique_public_ptr<T>(
            P::template static_cast_to<element_type>(p)
        ,   self
        );
    }
    
    template <typename T>
    typename detail::rma_typed_allocator_helper<P, T>::array_type
    make_unique_uninitialized(const size_type n)
    {
        using element_type = mefdn::remove_extent_t<T>;
        
        auto& self = this->derived();
        const auto size = n * sizeof(element_type);
        const auto p = self.untyped_allocate(size);
        
        return typename P::template unique_public_ptr<T>(
            P::template static_cast_to<element_type>(p)
        ,   self
        );
    }
};

} // namespace mecom2
} // namespace menps


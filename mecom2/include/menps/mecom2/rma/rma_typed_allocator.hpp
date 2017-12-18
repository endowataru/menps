
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom2 {

namespace detail {

template <typename P, typename T>
struct rma_typed_allocator_helper;

template <typename P, typename T>
struct rma_typed_allocator_helper<P, T []> {
    using array_type = typename P::template unique_local_ptr<T []>;
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
        
        return typename P::template unique_local_ptr<T>(
            P::template static_cast_to<element_type>(p)
        ,   self
        );
    }
    
    
    #if 0
    template <typename T>
    typename P::template unique_local_ptr<T>
    make_unique_uninit(size_type n)
    {
        
    }
    #endif
    
    
private:
    
};

} // namespace mecom2
} // namespace menps


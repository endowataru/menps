
#pragma once

#include "spmc_bounded_queue_base.hpp"

namespace mgbase {

template <typename T, mgbase::size_t Size>
class static_spmc_bounded_queue;

namespace detail {

template <typename T, mgbase::size_t Size>
struct static_spmc_bounded_queue_traits
{
    typedef static_spmc_bounded_queue<T, Size>  derived_type;
    typedef T               element_type;
    typedef mgbase::size_t  index_type;
};

} // namespace detail

template <typename T, mgbase::size_t Size>
class static_spmc_bounded_queue
    : public detail::spmc_bounded_queue_base<detail::static_spmc_bounded_queue_traits<T, Size>>
{
    typedef detail::static_spmc_bounded_queue_traits<T, Size>   traits_type;
    typedef detail::spmc_bounded_queue_base<traits_type>        base;
    
protected:
    typedef typename base::entry_type   entry_type;
    
public:
    static_spmc_bounded_queue() MGBASE_DEFAULT_NOEXCEPT = default;
    
    static mgbase::size_t capacity() MGBASE_NOEXCEPT {
        return Size;
    }
    
    entry_type& get_entry_at(const mgbase::size_t index) MGBASE_NOEXCEPT {
        MGBASE_ASSERT(index < Size);
        return arr_[index];
    }
    
private:
    entry_type arr_[Size];
};

} // namespace mgbase


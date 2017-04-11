
#pragma once

#include "mpsc_locked_bounded_queue_base.hpp"

namespace mgbase {

template <typename T, mgbase::size_t Size>
class static_mpsc_locked_bounded_queue;

namespace detail {

template <typename T, mgbase::size_t Size>
struct static_mpsc_locked_bounded_queue_traits
{
    typedef static_mpsc_locked_bounded_queue<T, Size>  derived_type;
    typedef T               element_type;
    typedef mgbase::size_t  index_type;
};

} // namespace detail

template <typename T, mgbase::size_t Size>
class static_mpsc_locked_bounded_queue
    : public detail::mpsc_locked_bounded_queue_base<detail::static_mpsc_locked_bounded_queue_traits<T, Size>>
{
    typedef detail::static_mpsc_locked_bounded_queue_traits<T, Size>   traits_type;
    typedef detail::mpsc_locked_bounded_queue_base<traits_type>        base;
    
protected:
    typedef typename base::entry_type   entry_type;
    
public:
    static_mpsc_locked_bounded_queue()
    {
        for (mgbase::size_t i = 0; i < Size; ++i)
            arr_[i].state.store(false);
    }
    
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


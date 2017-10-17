
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T, T... Ints>
struct integer_sequence
{
    typedef T   value_type;
    
    static MGBASE_CONSTEXPR mgbase::size_t size() MGBASE_NOEXCEPT {
        return sizeof...(Ints);
    }
};

namespace detail {

template <mgbase::size_t... Indexes>
struct make_index_sequence_chainer
{
    typedef integer_sequence<mgbase::size_t, Indexes...> type;
    
    typedef make_index_sequence_chainer<Indexes..., sizeof...(Indexes)> next_type;
};

template <mgbase::size_t N>
struct make_index_sequence_helper {
    typedef typename make_index_sequence_helper<N-1>::type::next_type  type;
};

template <>
struct make_index_sequence_helper<0> {
    typedef make_index_sequence_chainer<>   type;
};

} // namespace detail

#if 0
template <typename T, T N>
struct make_integer_sequence_type
{
    typedef typename detail::make_integer_sequence_helper<T, N>::type   type;
};
#endif

template <mgbase::size_t N>
struct make_index_sequence_type {
    typedef typename detail::make_index_sequence_helper<N>::type::type  type;
};
    //: make_integer_sequence_type<mgbase::size_t, N> { };


} // namespace mgbase


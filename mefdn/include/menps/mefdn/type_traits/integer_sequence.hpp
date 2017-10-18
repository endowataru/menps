
#pragma once

#include <menps/mefdn/lang.hpp>

namespace menps {
namespace mefdn {

template <typename T, T... Ints>
struct integer_sequence
{
    typedef T   value_type;
    
    static constexpr mefdn::size_t size() noexcept {
        return sizeof...(Ints);
    }
};

namespace detail {

template <mefdn::size_t... Indexes>
struct make_index_sequence_chainer
{
    typedef integer_sequence<mefdn::size_t, Indexes...> type;
    
    typedef make_index_sequence_chainer<Indexes..., sizeof...(Indexes)> next_type;
};

template <mefdn::size_t N>
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

template <mefdn::size_t N>
struct make_index_sequence_type {
    typedef typename detail::make_index_sequence_helper<N>::type::type  type;
};
    //: make_integer_sequence_type<mefdn::size_t, N> { };

template <mefdn::size_t... Ints>
using index_sequence = integer_sequence<mefdn::size_t, Ints...>;

template <mefdn::size_t N>
using make_index_sequence = typename detail::make_index_sequence_helper<N>::type::type;

template <typename... Ts>
using index_sequence_for = make_index_sequence<sizeof...(Ts)>;

} // namespace mefdn
} // namespace menps


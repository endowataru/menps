
#pragma once

#include "tuple.hpp"
#include <menps/mefdn/functional/invoke.hpp>
#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace mefdn {

namespace detail {

template <typename F, typename Tuple>
struct apply_seq
{
    typedef typename mefdn::make_index_sequence_type<
        mefdn::tuple_size<
            typename mefdn::decay<Tuple>::type
        >::value
    >::type
    type;
};

template <typename F, typename Tuple, typename Seq>
struct apply_result_helper;

template <typename F, typename Tuple, mefdn::size_t... Indexes>
struct apply_result_helper<F, Tuple, mefdn::integer_sequence<mefdn::size_t, Indexes...>>
    : mefdn::result_of<
        F (
            typename mefdn::tuple_element<
                Indexes
            ,   typename mefdn::decay<Tuple>::type
            >::type...
        )
    >
    { };

template <typename F, typename Tuple>
struct apply_result
    : apply_result_helper<F, Tuple, typename apply_seq<F, Tuple>::type> { };

template <typename F, typename Tuple, mefdn::size_t... Indexes>
inline typename apply_result<F, Tuple>::type
apply_impl(
    F&&      f
,   Tuple&&  t MEFDN_MAYBE_UNUSED
    // "t" is considered as "unused" in GCC when (sizeof...(Indexes) == 0)
,   mefdn::integer_sequence<mefdn::size_t, Indexes...> /*unused*/
)
{
    return mefdn::invoke(mefdn::forward<F>(f), mefdn::get<Indexes>(std::forward<Tuple>(t))...);
}

} // namespace detail

template <typename F, typename Tuple>
inline
typename detail::apply_result<F, Tuple>::type
apply(F&& f, Tuple&& t)
{
    return detail::apply_impl(
        mefdn::forward<F>(f)
    ,   mefdn::forward<Tuple>(t)
    ,   typename detail::apply_seq<F, Tuple>::type{}
    );
}

} // namespace mefdn
} // namespace menps


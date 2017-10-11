
#pragma once

#include "tuple.hpp"
#include <mgbase/type_traits/invoke.hpp>
#include <mgbase/type_traits/integer_sequence.hpp>
#include <mgbase/type_traits/decay.hpp>

#include <mgbase/type_traits/result_of.hpp>

namespace mgbase {

namespace detail {

template <typename F, typename Tuple>
struct apply_seq
{
    typedef typename mgbase::make_index_sequence_type<
        mgbase::tuple_size<
            typename mgbase::decay<Tuple>::type
        >::value
    >::type
    type;
};

template <typename F, typename Tuple, typename Seq>
struct apply_result_helper;

template <typename F, typename Tuple, mgbase::size_t... Indexes>
struct apply_result_helper<F, Tuple, mgbase::integer_sequence<mgbase::size_t, Indexes...>>
    : mgbase::result_of<
        F (
            typename mgbase::tuple_element<
                Indexes
            ,   typename mgbase::decay<Tuple>::type
            >::type...
        )
    >
    { };

template <typename F, typename Tuple>
struct apply_result
    : apply_result_helper<F, Tuple, typename apply_seq<F, Tuple>::type> { };

template <typename F, typename Tuple, mgbase::size_t... Indexes>
inline typename apply_result<F, Tuple>::type
apply_impl(
    F&&      f
,   Tuple&&  t MGBASE_UNUSED
    // "t" is considered as "unused" in GCC when (sizeof...(Indexes) == 0)
,   mgbase::integer_sequence<mgbase::size_t, Indexes...> /*unused*/
)
{
    return mgbase::invoke(mgbase::forward<F>(f), mgbase::get<Indexes>(std::forward<Tuple>(t))...);
}

} // namespace detail

template <typename F, typename Tuple>
inline
typename detail::apply_result<F, Tuple>::type
apply(F&& f, Tuple&& t)
{
    return detail::apply_impl(
        mgbase::forward<F>(f)
    ,   mgbase::forward<Tuple>(t)
    ,   typename detail::apply_seq<F, Tuple>::type{}
    );
}

} // namespace mgbase


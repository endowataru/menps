
#pragma once

#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/utility.hpp>

namespace menps {
namespace mefdn {

// Sequential version for "parallel for" loop
// defined in P0075r2.

template <typename I, typename S, typename Func>
void for_loop_strided(
    type_identity_t<I>  start
,   I                   finish
,   S                   stride
,   Func                func
    // TODO: Implement reduction & induction.
) {
    for ( ; start < finish ; start += stride) {
        func(start);
    }
}

template <typename I, typename... Rest>
void for_loop(
    const type_identity_t<I>    start
,   const I                     finish
,   Rest && ...                 rest
) {
    const I stride = 1;
    for_loop_strided(start, finish, stride,
        mefdn::forward<Rest>(rest)...);
}

} // namespace mefdn
} // namespace menps


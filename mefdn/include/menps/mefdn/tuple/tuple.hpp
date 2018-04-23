
#pragma once

#include <menps/mefdn/lang.hpp>
#include <tuple>
#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/utility.hpp>

namespace menps {
namespace mefdn {

using std::tuple;
using std::make_tuple;
using std::tuple_size;
using std::tuple_element;
using std::get;
using std::forward_as_tuple;


// Non-standard function to wrap "void" as tuple

template <typename Func, typename... Args>
inline typename mefdn::enable_if<
    // return type is void
    ! mefdn::is_void<
        typename mefdn::result_of<Func (Args...)>::type
    >::value
,   mefdn::tuple<typename mefdn::result_of<Func (Args...)>::type>
>::type
call_and_make_tuple(Func&& func, Args&&... args)
{
    return mefdn::forward_as_tuple(
        mefdn::forward<Func>(func)(
            mefdn::forward<Args>(args)...
        )
    );
}

template <typename Func, typename... Args>
inline typename mefdn::enable_if<
    // return type is void
    mefdn::is_void<
        typename mefdn::result_of<Func (Args...)>::type
    >::value
,   mefdn::tuple<>
>::type
call_and_make_tuple(Func&& func, Args&&... args)
{
    mefdn::forward<Func>(func)(
        mefdn::forward<Args>(args)...
    );
    return mefdn::make_tuple();
}

} // namespace mefdn
} // namespace menps


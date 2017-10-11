
#pragma once

#include <tuple>
#include <mgbase/utility/forward.hpp>
#include <mgbase/type_traits/add_const.hpp>
#include <mgbase/type_traits/add_volatile.hpp>
#include <mgbase/type_traits/add_cv.hpp>

namespace mgbase {

using std::tuple;

using std::make_tuple;


// GCC 4.4 doesn't have tuple_element

//using std::tuple_elemennt;

template <mgbase::size_t Index, typename T>
struct tuple_element;

template <mgbase::size_t Index, typename T>
struct tuple_element<Index, const T> {
    typedef typename mgbase::add_const<
        typename tuple_element<Index, T>::type
    >::type type;
};

template <mgbase::size_t Index, typename T>
struct tuple_element<Index, volatile T> {
    typedef typename mgbase::add_volatile<
        typename tuple_element<Index, T>::type
    >::type type;
};
template <mgbase::size_t Index, typename T>
struct tuple_element<Index, const volatile T > {
    typedef typename mgbase::add_cv<
        typename tuple_element<Index, T>::type
    >::type type;
};

template <mgbase::size_t Index, typename Head, typename... Tail>
struct tuple_element<Index, tuple<Head, Tail...>>
    : tuple_element<Index-1, tuple<Tail...>> { };
 
template <typename Head, typename... Tail>
struct tuple_element<0, tuple<Head, Tail...>> {
    typedef Head type;
};


using std::tuple_size;

template <typename... Args>
inline tuple<Args&&...> forward_as_tuple(Args&&... args) MGBASE_NOEXCEPT {
    return tuple<Args&&...>(mgbase::forward<Args>(args)...);
}

using std::get;

} // namespace mgbase


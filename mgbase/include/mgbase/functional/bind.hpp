
#pragma once

#include <functional>
#include <mgbase/type_traits/integral_constant.hpp>
#include <mgbase/type_traits/enable_if.hpp>
#include <mgbase/type_traits/decay.hpp>
#include <mgbase/type_traits/integer_sequence.hpp>
#include <mgbase/type_traits/invoke.hpp>
#include <mgbase/type_traits/result_of.hpp>
#include <mgbase/utility/declval.hpp>
#include <mgbase/tuple/tuple.hpp> // mgbase::get

namespace mgbase {

namespace detail {

template <int Num>
struct placeholder { };

} // namespace detail

namespace placeholders {

extern detail::placeholder<1> _1;
extern detail::placeholder<2> _2;
extern detail::placeholder<3> _3;
extern detail::placeholder<4> _4;
extern detail::placeholder<5> _5;

} // namespace placeholders

template <typename T>
struct is_placeholder
    : mgbase::integral_constant<int, 0> { };

template <int Num>
struct is_placeholder<detail::placeholder<Num>>
    : mgbase::integral_constant<int, Num> { };

namespace detail {

template <typename Bound>
struct is_unbound
    : is_placeholder<typename mgbase::decay<Bound>::type> { };

template <typename Bound, typename Unbound,
    int IsUnbound = is_unbound<Bound>::value>
struct bind_arg
{
    typedef typename mgbase::tuple_element<
        IsUnbound - 1
    ,   typename mgbase::decay<Unbound>::type
    >::type type;
    
    static type get(Bound /*bound*/, Unbound unbound)
    {
        return mgbase::get<IsUnbound - 1>(unbound);
    }
};

template <typename Bound, typename Unbound>
struct bind_arg<Bound, Unbound, 0>
{
    typedef Bound type;
    
    static type get(Bound bound, Unbound /*unbound*/)
    {
        return mgbase::forward<Bound>(bound);
    }
};

template <typename Bound, typename Unbound,
    // Make an index sequence to expand parameters
    typename = typename make_index_sequence_type<
        mgbase::tuple_size<typename mgbase::decay<Bound>::type>::value
    >::type>
struct bind_result_expand;

template <typename Bound, typename Unbound, mgbase::size_t... Indexes>
struct bind_result_expand<Bound, Unbound, integer_sequence<mgbase::size_t, Indexes...>>
{
private:
    typedef typename mgbase::decay<Bound>::type bound_type;
    
    // Get the type of Nth bind_arg.
    template <mgbase::size_t N>
    struct bind_arg_of
        : bind_arg<
            typename mgbase::tuple_element<N, bound_type>::type
        ,   Unbound
        > { };
    
public:
    typedef typename mgbase::invoke_of<
        typename bind_arg_of<Indexes>::type...
    >::type type;
    
    static type call(Bound&& bound, Unbound&& unbound) {
        return mgbase::invoke(
            bind_arg_of<Indexes>::get(
                mgbase::get<Indexes>(mgbase::forward<Bound>(bound))
            ,   mgbase::forward<Unbound>(unbound)
            )...
        );
    }
};

template <typename Bound, typename Unbound>
struct bind_result
    : bind_result_expand<Bound, Unbound> { };

template <typename F, typename... Args>
class bind_
{
    typedef mgbase::tuple<F, Args...>   tuple_type;
    
public:
    template <typename... Bound>
    explicit bind_(Bound&&... bound)
        : bound_(bound...)
        { }
    
    template <typename... Unbound>
    typename bind_result<tuple_type&, mgbase::tuple<Unbound...>&&>::type
    operator() (Unbound&&... unbound)
    {
        return bind_result<tuple_type&, mgbase::tuple<Unbound...>&&>
            ::call(
                bound_
            ,   mgbase::tuple<Unbound...>(mgbase::forward<Unbound>(unbound)...)
            );
    }
    
private:
    tuple_type bound_;
};

} // namespace detail

template <typename F, typename... Args>
detail::bind_<F, Args...> bind(F&& f, Args&&... args)
{
    return detail::bind_<F, Args...>(
        mgbase::forward<F>(f)
    ,   mgbase::forward<Args>(args)...
    );
}

#if 0
namespace placeholders = std::placeholders;

using std::bind;
#endif


} // namespace mgbase


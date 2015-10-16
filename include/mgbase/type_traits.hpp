
#pragma once

#include "lang.hpp"

#if (__cplusplus < 201103L)

namespace mgbase {

template <bool B, typename T = void>
struct enable_if;

template <typename T>
struct enable_if<true, T>
{
    typedef T   type;
};


template <typename T>
struct identity {
    typedef T   type;
};

template <typename T>
T declval() MGBASE_NOEXCEPT; // not defined

template <typename T, T v>
struct integral_constant
{
    static const T value = v;
    
    typedef T                   value_type;
    typedef integral_constant   type;
    
    operator value_type() const MGBASE_NOEXCEPT { return value; }
};

typedef integral_constant<bool, true>   true_type;
typedef integral_constant<bool, false>  false_type;

template <typename T, typename U>
struct is_same : false_type {};

template <typename T>
struct is_same<T, T> : true_type {};
 
template <typename T> struct remove_const          { typedef T type; };
template <typename T> struct remove_const<const T> { typedef T type; };
 
template <typename T> struct remove_volatile             { typedef T type; };
template <typename T> struct remove_volatile<volatile T> { typedef T type; };

template <typename T>
struct remove_cv {
    typedef typename remove_volatile<typename remove_const<T>::type>::type type;
};

template <typename T >
struct is_void : is_same<void, typename remove_cv<T>::type> { };

template <typename> struct is_integral             : false_type { };
template <> struct is_integral<bool>               : true_type { };
template <> struct is_integral<char>               : true_type { };
template <> struct is_integral<signed char>        : true_type { };
template <> struct is_integral<wchar_t>            : true_type { };
template <> struct is_integral<short>              : true_type { };
template <> struct is_integral<int>                : true_type { };
template <> struct is_integral<long>               : true_type { };
template <> struct is_integral<long long>          : true_type { };
template <> struct is_integral<unsigned char>      : true_type { };
template <> struct is_integral<unsigned short>     : true_type { };
template <> struct is_integral<unsigned int>       : true_type { };
template <> struct is_integral<unsigned long>      : true_type { };
template <> struct is_integral<unsigned long long> : true_type { };

template <typename> struct is_floating_point      : false_type { };
template <> struct is_floating_point<float>       : true_type { };
template <> struct is_floating_point<double>      : true_type { };
template <> struct is_floating_point<long double> : true_type { };

template <typename T>
struct is_arithmetic
    : integral_constant<bool,
        is_integral<T>::value || is_floating_point<T>::value
    > { };

template <typename T>
struct is_fundamental
    : integral_constant<bool,
        is_arithmetic<T>::value || is_void<T>::value
    > { };

template <typename T>
struct is_compound
    : integral_constant<bool,
        !is_fundamental<T>::value
    > { };


template <typename T>
struct is_array
    : false_type { };

template <typename T>
struct is_array<T []>
    : true_type { };

template <typename T, std::size_t S>
struct is_array<T [S]>
    : true_type { };


// is_convertible
// From Boost.TypeTraits

namespace detail {

struct yes_type { char x;    };
struct no_type  { char x[2]; };

struct any_conversion
{
    template <typename T> any_conversion(const volatile T&);
    template <typename T> any_conversion(const T&);
    template <typename T> any_conversion(volatile T&);
    template <typename T> any_conversion(T&);
};

template <typename From, typename To>
struct is_convertible_impl
{
    static no_type  check(any_conversion, ...);
    static yes_type check(To, int);
    
    static const bool value = sizeof(check(declval<From>())) == sizeof(yes_type);
};

}

template <typename From, typename To>
struct is_convertible
{
    static const bool value = detail::is_convertible_impl<From, To>::value;
};

}

#else

#include <type_traits>

namespace mgbase {

using std::enable_if;

using std::is_fundamental;

}

#endif


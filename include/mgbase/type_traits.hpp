
#pragma once

#include "lang.hpp"

#if (__cplusplus < 201103L)

namespace mgbase {

// enable_if

template <bool B, typename T = void>
struct enable_if;

template <typename T>
struct enable_if<true, T>
{
    typedef T   type;
};

// identity (not included in C++ standard)

template <typename T>
struct identity {
    typedef T   type;
};

// declval

template <typename T>
T declval() MGBASE_NOEXCEPT; // not defined

// integral_constant

template <typename T, T v>
struct integral_constant
{
    static const T value = v;
    
    typedef T                   value_type;
    typedef integral_constant   type;
    
    operator value_type() const MGBASE_NOEXCEPT { return value; }
};

// bool_constant

template <bool B>
struct bool_constant : integral_constant<bool, B> { };

typedef bool_constant<true>   true_type;
typedef bool_constant<false>  false_type;

// is_same

template <typename T, typename U>
struct is_same : false_type {};

template <typename T>
struct is_same<T, T> : true_type {};

// remove_const

template <typename T> struct remove_const          { typedef T type; };
template <typename T> struct remove_const<const T> { typedef T type; };

// remove_volatile

template <typename T> struct remove_volatile             { typedef T type; };
template <typename T> struct remove_volatile<volatile T> { typedef T type; };

// remove_cv

template <typename T>
struct remove_cv {
    typedef typename remove_volatile<typename remove_const<T>::type>::type type;
};

// is_const

template <typename T> struct is_const          : false_type {};
template <typename T> struct is_const<const T> : true_type  {};

// is_void

template <typename T >
struct is_void : is_same<void, typename remove_cv<T>::type> { };

// is_integral

namespace detail {

template <typename> struct is_integral_helper             : false_type { };
template <> struct is_integral_helper<bool>               : true_type { };
template <> struct is_integral_helper<char>               : true_type { };
template <> struct is_integral_helper<signed char>        : true_type { };
template <> struct is_integral_helper<wchar_t>            : true_type { };
template <> struct is_integral_helper<short>              : true_type { };
template <> struct is_integral_helper<int>                : true_type { };
template <> struct is_integral_helper<long>               : true_type { };
template <> struct is_integral_helper<long long>          : true_type { };
template <> struct is_integral_helper<unsigned char>      : true_type { };
template <> struct is_integral_helper<unsigned short>     : true_type { };
template <> struct is_integral_helper<unsigned int>       : true_type { };
template <> struct is_integral_helper<unsigned long>      : true_type { };
template <> struct is_integral_helper<unsigned long long> : true_type { };

} // namespace detail

template <typename T>
struct is_integral
    : detail::is_integral_helper<typename remove_cv<T>::type> { };

// is_floating_point

namespace detail {

template <typename> struct is_floating_point_helper      : false_type { };
template <> struct is_floating_point_helper<float>       : true_type { };
template <> struct is_floating_point_helper<double>      : true_type { };
template <> struct is_floating_point_helper<long double> : true_type { };

} // namespace detail

template <typename T>
struct is_floating_point
    : detail::is_floating_point_helper<typename remove_cv<T>::type> { };

// is_arithmetic

template <typename T>
struct is_arithmetic
    : bool_constant<
        is_integral<T>::value || is_floating_point<T>::value
    > { };

// is_fundamental

template <typename T>
struct is_fundamental
    : bool_constant<
        is_arithmetic<T>::value || is_void<T>::value
    > { };

// is_compound

template <typename T>
struct is_compound
    : bool_constant<
        !is_fundamental<T>::value
    > { };

// is_array

template <typename T>
struct is_array
    : false_type { };

template <typename T>
struct is_array<T []>
    : true_type { };

template <typename T, std::size_t S>
struct is_array<T [S]>
    : true_type { };

// is_pointer

namespace detail {

template <typename T> struct is_pointer_helper     : false_type { };
template <typename T> struct is_pointer_helper<T*> : true_type  { };

} // namespace detail

template <typename T>
struct is_pointer
    : detail::is_pointer_helper<typename remove_cv<T>::type> { };

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
    
    static const bool value = sizeof(check(declval<From>(), 0)) == sizeof(yes_type);
};

} // namespace detail

template <typename From, typename To>
struct is_convertible
    : bool_constant<detail::is_convertible_impl<From, To>::value> { };

// TODO : move to the unit test
MGBASE_STATIC_ASSERT((mgbase::is_convertible<int*, const int*>::value));

// is_union
// based on compiler intrinsic

template <typename T>
struct is_union
    : bool_constant<__is_union(T)> { };

// is_class

namespace detail {

template <typename T> yes_type is_class_helper(int T::*);
template <typename T> no_type is_class_helper();

} // namespace detail

template <typename T>
struct is_class
    : bool_constant<
        (sizeof(detail::is_class_helper<T>(0)) == sizeof(detail::yes_type))
    &&  ! is_union<T>::value
    > { };

// conditional

template <bool Condition, typename True, typename False>
struct conditional { typedef True type; };

template <typename True, typename False>
struct conditional<false, True, False> { typedef False type; };

// alignment_of

namespace detail {

template <typename T, T x, T y>
struct compile_time_min
    : integral_constant<T, x < y ? x : y> { };

template <typename T>
struct alignment_hack
{
    char c;
    T t;
};

} // namespace detail

// See also:
//  - Boost.TypeTraits
//  - http://d.hatena.ne.jp/Cryolite/20051102#p2

template <typename T>
struct alignment_of
    : detail::compile_time_min<
        mgbase::size_t
    ,   sizeof(detail::alignment_hack<T>) - sizeof(T)
    ,   sizeof(T)
    >
    { };

// aligned_storage

namespace detail {

struct dummy_t;

union max_align {
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
    long double ld;
    void* vp;
    void (*fp)();
    int dummy_t::*mp;
    void (dummy_t::*mfp)();
};

} // namespace detail

// See also:
//  - http://qiita.com/okdshin/items/8afb5e3e6044798e162c

template <mgbase::size_t Size, mgbase::size_t Align>
class aligned_storage
{
    typedef detail::max_align   align_type;
    
    MGBASE_STATIC_ASSERT(alignment_of<align_type>::value >= Align);
    MGBASE_STATIC_ASSERT(alignment_of<align_type>::value % Align == 0);
    
public:
    union type {
        char        arr[Size];
        align_type  al;
    };
};

// make_signed

template <typename T> struct make_signed;
template <> struct make_signed<char>               { typedef signed char        type; };
template <> struct make_signed<signed char>        { typedef signed char        type; };
template <> struct make_signed<short>              { typedef signed short       type; };
template <> struct make_signed<int>                { typedef signed int         type; };
template <> struct make_signed<long>               { typedef signed long        type; };
template <> struct make_signed<long long>          { typedef signed long long   type; };
template <> struct make_signed<unsigned char>      { typedef signed char        type; };
template <> struct make_signed<unsigned short>     { typedef signed short       type; };
template <> struct make_signed<unsigned int>       { typedef signed int         type; };
template <> struct make_signed<unsigned long>      { typedef signed long        type; };
template <> struct make_signed<unsigned long long> { typedef signed long long   type; };

template <typename T> struct make_signed<const T> {
    typedef const typename make_signed<T>::type type;
};
template <typename T> struct make_signed<volatile T> {
    typedef volatile typename make_signed<T>::type type;
};
template <typename T> struct make_signed<const volatile T> {
    typedef const volatile typename make_signed<T>::type type;
};

// make_unsigned

template <typename T> struct make_unsigned;
template <> struct make_unsigned<char>               { typedef unsigned char            type; };
template <> struct make_unsigned<signed char>        { typedef unsigned char            type; };
template <> struct make_unsigned<short>              { typedef unsigned short           type; };
template <> struct make_unsigned<int>                { typedef unsigned int             type; };
template <> struct make_unsigned<long>               { typedef unsigned long            type; };
template <> struct make_unsigned<long long>          { typedef unsigned long long       type; };
template <> struct make_unsigned<unsigned char>      { typedef unsigned char            type; };
template <> struct make_unsigned<unsigned short>     { typedef unsigned short           type; };
template <> struct make_unsigned<unsigned int>       { typedef unsigned int             type; };
template <> struct make_unsigned<unsigned long>      { typedef unsigned long            type; };
template <> struct make_unsigned<unsigned long long> { typedef unsigned long long       type; };

template <typename T> struct make_unsigned<const T> {
    typedef const typename make_unsigned<T>::type type;
};
template <typename T> struct make_unsigned<volatile T> {
    typedef volatile typename make_unsigned<T>::type type;
};
template <typename T> struct make_unsigned<const volatile T> {
    typedef const volatile typename make_unsigned<T>::type type;
};


#if 0

namespace detail {

template <mgbase::size_t> struct sizeof_trick;

template <typename F> struct is_callable_impl;

template <typename Func>
struct is_callable_impl<Func ()>
{
private:
    template <typename F>
    static yes_type check(sizeof_trick<sizeof(
        mgbase::declval<F>()(), 0
    )>*);
    template <typename F>
    static no_type check(...);

public:
    static const bool value = sizeof(check<Func>(0)) == sizeof(yes_type);
};

template <typename Func, typename Arg1>
struct is_callable_impl<Func (Arg1)>
{
private:
    template <typename F, typename A1>
    static yes_type check(sizeof_trick<sizeof(
        mgbase::declval<F>()(mgbase::declval<A1>()), 0
    )>*);
    template <typename F, typename A1>
    static no_type check(...);

public:
    static const bool value = sizeof(check<Func, Arg1>(0)) == sizeof(yes_type);
};

} // namespace detail

template <typename F>
struct is_callable
    : bool_constant<detail::is_callable_impl<F>::value> { };


#endif

} // namespace mgbase

#else

#include <type_traits>

namespace mgbase {

using std::enable_if;

using std::is_fundamental;

}

#endif


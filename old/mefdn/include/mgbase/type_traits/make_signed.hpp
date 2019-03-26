
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

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

} // namespace mgbase


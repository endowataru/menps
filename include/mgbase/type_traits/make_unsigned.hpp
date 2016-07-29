
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

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

} // namespace mgbase


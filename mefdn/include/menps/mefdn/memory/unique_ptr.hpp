
#pragma once

#include <menps/mefdn/type_traits.hpp>
#include <memory>

namespace menps {
namespace mefdn {

using std::default_delete;
using std::unique_ptr;


// make_unique

namespace detail {

template <typename T>
struct make_unique_helper
{
    typedef unique_ptr<T>       single_type;
};
template <typename T>
struct make_unique_helper<T []>
{
    typedef unique_ptr<T []>    array_type;
};
template <typename T, mefdn::size_t Size>
struct make_unique_helper<T [Size]>
{
    struct invalid_type;
};

} // namespace detail

// Single-object

template <typename T, typename... Args>
inline typename detail::make_unique_helper<T>::single_type
make_unique(Args&&... args)
{
    return unique_ptr<T>(
        new T(std::forward<Args>(args)...)
    );
}

// Array (variable-length)

template <typename T>
inline typename detail::make_unique_helper<T>::array_type
make_unique(const mefdn::size_t size)
{
    typedef typename remove_extent<T>::type element_type;
    
    return unique_ptr<T>(
        new element_type[size]()
    );
}

// Array (fixed-size)

template <typename T, typename... Args>
inline typename detail::make_unique_helper<T>::invalid_type
make_unique(Args&&...) = delete;




// Non-standard functions to allocate an array without initialization

template <typename T>
inline typename detail::make_unique_helper<T>::array_type
make_unique_uninitialized(const mefdn::size_t size)
{
    using element_type = remove_extent_t<T>;
    
    return unique_ptr<T>(
        // No value-initialization here.
        new element_type[size]
    );
}

} // namespace mefdn
} // namespace menps


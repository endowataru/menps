
#pragma once

#include <menps/meth/common.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/medsm/global_variable.hpp>

namespace menps {
namespace meth {

namespace dsm {

namespace untyped {

void* allocate(mefdn::size_t alignment, mefdn::size_t size_in_bytes);

void deallocate(void*);

} // namespace untyped

namespace detail {

template <typename T>
struct construct_result {
    typedef T*      single_type;
    typedef void    void_type;
};
template <typename T>
struct construct_result<T []> {
    typedef T*      array_type;
    typedef void    void_type;
};
template <typename T, mefdn::size_t Size>
struct construct_result<T [Size]> {
    struct invalid_type;
    struct void_type;
};

} // namespace detail

template <typename T, typename... Args>
inline typename detail::construct_result<T>::single_type
construct(Args&&... args)
{
    auto p = untyped::allocate(alignof(T), sizeof(T));
    
    return new (p) T(mefdn::forward<Args>(args)...);
}
template <typename T>
inline typename detail::construct_result<T>::array_type
construct(const mefdn::size_t size)
{
    typedef typename mefdn::remove_extent<T>::type element_type;
    
    auto p = untyped::allocate(
        alignof(element_type)
    ,   sizeof(element_type) * size
    );
    
    return new (p) element_type[size]();
}
template <typename T>
inline typename detail::construct_result<T>::invalid_type
construct() = delete;

template <typename T>
inline void destruct(const typename detail::construct_result<T>::single_type ptr)
{
    ptr->~T();
    
    untyped::deallocate(ptr);
}
template <typename T>
inline void destruct(const typename detail::construct_result<T>::array_type ptr)
{
    // FIXME: call destructor for array
    
    untyped::deallocate(ptr);
}
template <typename T>
inline void destruct(typename detail::construct_result<T>::invalid_type) = delete;


template <typename T>
struct single_delete
{
    void operator() (T* const p)
    {
        meth::dsm::destruct(p);
    }
};
template <typename T>
struct array_delete
{
    void operator() (T* const p)
    {
        meth::dsm::destruct<T []>(p);
    }
};

namespace detail {

template <typename T>
struct make_unique_helper
{
    typedef mefdn::unique_ptr<T, single_delete<T>>     single_type;
};
template <typename T>
struct make_unique_helper<T []>
{
    typedef mefdn::unique_ptr<T [], array_delete<T>>   array_type;
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
    return typename detail::make_unique_helper<T>::single_type(
        meth::dsm::construct(std::forward<Args>(args)...)
    );
}

// Array (variable-length)

template <typename T>
inline typename detail::make_unique_helper<T>::array_type
make_unique(const mefdn::size_t size)
{
    return typename detail::make_unique_helper<T>::array_type(
        meth::dsm::construct<T>(size)
    );
}

// Array (fixed-size)

template <typename T, typename... Args>
inline typename detail::make_unique_helper<T>::invalid_type
make_unique(Args&&...) = delete;


// malloc/free interface

inline void* malloc(const mefdn::size_t size) {
    return untyped::allocate(16 /*TODO*/, size);
}

inline void free(void* const ptr) {
    untyped::deallocate(ptr);
}

} // namespace dsm

} // namespace meth
} // namespace menps

// Provide an alias.
#define METH_GLOBAL_VARIABLE    MEDSM_GLOBAL_VARIABLE


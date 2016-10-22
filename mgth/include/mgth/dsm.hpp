
#pragma once

#include <mgbase/lang.hpp>
#include <mgbase/unique_ptr.hpp>
#include <mgbase/type_traits/remove_extent.hpp>
#include <mgbase/utility/forward.hpp>

namespace mgth {

namespace dsm {

namespace untyped {

void* allocate(mgbase::size_t alignment, mgbase::size_t size_in_bytes);

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
template <typename T, mgbase::size_t Size>
struct construct_result<T [Size]> {
    struct invalid_type;
    struct void_type;
};

} // namespace detail

template <typename T, typename... Args>
inline typename detail::construct_result<T>::single_type
construct(Args&&... args)
{
    auto p = untyped::allocate(MGBASE_ALIGNOF(T), sizeof(T));
    
    return new (p) T(mgbase::forward<Args>(args)...);
}
template <typename T>
inline typename detail::construct_result<T>::array_type
construct(const mgbase::size_t size)
{
    typedef typename mgbase::remove_extent<T>::type element_type;
    
    auto p = untyped::allocate(
        MGBASE_ALIGNOF(element_type)
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
        mgth::dsm::destruct(p);
    }
};
template <typename T>
struct array_delete
{
    void operator() (T* const p)
    {
        mgth::dsm::destruct<T []>(p);
    }
};

namespace detail {

template <typename T>
struct make_unique_helper
{
    typedef mgbase::unique_ptr<T, single_delete<T>>     single_type;
};
template <typename T>
struct make_unique_helper<T []>
{
    typedef mgbase::unique_ptr<T [], array_delete<T>>   array_type;
};
template <typename T, mgbase::size_t Size>
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
        mgth::dsm::construct(std::forward<Args>(args)...)
    );
}

// Array (variable-length)

template <typename T>
inline typename detail::make_unique_helper<T>::array_type
make_unique(const mgbase::size_t size)
{
    return typename detail::make_unique_helper<T>::array_type(
        mgth::dsm::construct<T>(size)
    );
}

// Array (fixed-size)

template <typename T, typename... Args>
inline typename detail::make_unique_helper<T>::invalid_type
make_unique(Args&&...) = delete;

} // namespace dsm

} // namespace mgth


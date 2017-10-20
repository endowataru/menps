
#pragma once

#include <menps/mefdn/lang.hpp>
#include "pointer_facade.hpp"
#include <algorithm>

namespace menps {
namespace mefdn {

// runtime_sized_traits

typedef std::size_t runtime_size_t;

namespace detail {

template <typename T>
struct runtime_sized_traits {
    static constexpr runtime_size_t size() noexcept {
        return sizeof(T);
    }
};

}

template <typename T>
struct runtime_sized_traits
    : detail::runtime_sized_traits<
        // Examine the alias first.
        typename mefdn::get_actual_type<T>::type
    > { };

// helper function to calculate the size
template <typename T>
inline constexpr runtime_size_t runtime_size_of() noexcept {
    return runtime_sized_traits<T>::size();
}

namespace detail {

template <typename T, std::size_t S>
struct runtime_sized_traits<T [S]> {
    static runtime_size_t size() noexcept {
        return runtime_size_of<T>() * S;
    }
};

}

// Runtime-sized classes
// These classes are incomplete types.

template <runtime_size_t (*Size)()>
struct runtime_sized_struct;

template <typename T, runtime_size_t (*Size)()>
struct runtime_sized_array;

template <typename T1, typename T2>
struct runtime_sized_pair;

template <typename T1, typename T2>
struct runtime_sized_union_pair;

namespace detail {

template <runtime_size_t (*Size)()>
struct runtime_sized_traits< runtime_sized_struct<Size> > {
    static constexpr runtime_size_t size() noexcept {
        return Size();
    }
};

template <typename T, runtime_size_t (*Size)()>
struct runtime_sized_traits< runtime_sized_array<T, Size> > {
    static runtime_size_t size() noexcept {
        return runtime_size_of<T>() * Size();
    }
};

template <typename T1, typename T2>
struct runtime_sized_traits< runtime_sized_pair<T1, T2> > {
    static runtime_size_t size() noexcept {
        return runtime_size_of<T1>() + runtime_size_of<T2>();
    }
};

template <typename T1, typename T2>
struct runtime_sized_traits< runtime_sized_union_pair<T1, T2> > {
    static runtime_size_t size() noexcept {
        return std::max(runtime_size_of<T1>(), runtime_size_of<T2>());
    }
};

}

// runtime_sized meta functions

template <typename To, typename From>
struct is_runtime_sized_assignable
    : mefdn::integral_constant<bool,
        // To is not const
        !mefdn::is_const<To>::value
        // To is the same type as From
        // TODO: This is too restrictive
        && mefdn::is_same<To, typename mefdn::remove_const<From>::type>::value
    > { };

// get functions for runtime_sized_pair & runtime_sized_union_pair

namespace detail {

template <typename T1, typename T2>
struct runtime_sized_pair_traits
{
    typedef T1  first_type;
    typedef T2  second_type;
    
    template <template <typename> class Derived, typename T>
    static Derived<first_type> get_first(const pointer_facade<Derived, T>& ptr) {
        return mefdn::reinterpret_pointer_cast<first_type>(ptr);
    }
    template <template <typename> class Derived, typename T>
    static Derived<second_type> get_second(const pointer_facade<Derived, T>& ptr) {
        return mefdn::reinterpret_pointer_cast<second_type>(get_first(ptr) + 1);
    }
};

template <typename T1, typename T2>
struct runtime_sized_union_pair_traits
{
    typedef T1  first_type;
    typedef T2  second_type;
    
    template <template <typename> class Derived, typename T>
    static Derived<first_type> get_first(const pointer_facade<Derived, T>& ptr) {
        return mefdn::reinterpret_pointer_cast<first_type>(ptr);
    }
    template <template <typename> class Derived, typename T>
    static Derived<second_type> get_second(const pointer_facade<Derived, T>& ptr) {
        return mefdn::reinterpret_pointer_cast<second_type>(ptr);
    }
};

}

template <typename T>
struct runtime_sized_pair_like_traits
    : runtime_sized_pair_like_traits<typename mefdn::get_actual_type<T>::type>
    { };

template <typename T1, typename T2>
struct runtime_sized_pair_like_traits< runtime_sized_pair<T1, T2> >
    : detail::runtime_sized_pair_traits<T1, T2> { };

template <typename T1, typename T2>
struct runtime_sized_pair_like_traits< const runtime_sized_pair<T1, T2> >
    : detail::runtime_sized_pair_traits<const T1, const T2> { };

template <typename T1, typename T2>
struct runtime_sized_pair_like_traits< runtime_sized_union_pair<T1, T2> >
    : detail::runtime_sized_union_pair_traits<T1, T2> { };

template <typename T1, typename T2>
struct runtime_sized_pair_like_traits< const runtime_sized_union_pair<T1, T2> >
    : detail::runtime_sized_union_pair_traits<const T1, const T2> { };


template <template <typename> class Derived, typename T>
inline Derived<
    typename runtime_sized_pair_like_traits<T>::first_type
>
get_first(const pointer_facade<Derived, T>& ptr)
{
    return runtime_sized_pair_like_traits<T>::get_first(ptr);
}
template <template <typename> class Derived, typename T>
inline Derived<
    typename runtime_sized_pair_like_traits<T>::second_type
>
get_second(const pointer_facade<Derived, T>& ptr)
{
    return runtime_sized_pair_like_traits<T>::get_second(ptr);
}


// get_element_at

namespace detail {

template <typename T, runtime_size_t (*Size)()>
struct runtime_sized_array_traits {
    typedef T   element_type;
    
    template <template <typename> class Derived, typename U>
    static Derived<element_type> get_element_at(const pointer_facade<Derived, U>& ptr, const std::size_t index) {
        MEFDN_ASSERT(index < Size());
        return mefdn::reinterpret_pointer_cast<element_type>(ptr) + static_cast<mefdn::ptrdiff_t>(index);
    }
};

template <typename T, std::size_t Size>
struct statically_sized_array_traits {
    typedef T   element_type;
    
    template <template <typename> class Derived, typename U>
    static Derived<element_type> get_element_at(const pointer_facade<Derived, U>& ptr, const std::size_t index) {
        MEFDN_ASSERT(index < Size);
        return mefdn::reinterpret_pointer_cast<element_type>(ptr) + static_cast<mefdn::ptrdiff_t>(index);
    }
};

}

template <typename T>
struct runtime_sized_array_like_traits
    : runtime_sized_array_like_traits<
        typename mefdn::get_actual_type<T>::type
    > { };

template <typename T, runtime_size_t (*Size)()>
struct runtime_sized_array_like_traits< runtime_sized_array<T, Size> >
    : detail::runtime_sized_array_traits<T, Size> { };

template <typename T, runtime_size_t (*Size)()>
struct runtime_sized_array_like_traits< const runtime_sized_array<T, Size> >
    : detail::runtime_sized_array_traits<const T, Size> { };

template <typename T, std::size_t Size>
struct runtime_sized_array_like_traits< T [Size] >
    : detail::statically_sized_array_traits<T, Size> { };

template <typename T, std::size_t Size>
struct runtime_sized_array_like_traits< const T[Size] >
    : detail::statically_sized_array_traits<const T, Size> { };

template <template <typename> class Derived, typename T>
inline Derived<
    typename runtime_sized_array_like_traits<T>::element_type
>
get_element_at(const pointer_facade<Derived, T>& ptr, std::size_t index) {
    return runtime_sized_array_like_traits<T>::get_element_at(ptr, index);
}



// runtime_sized_*

template <typename T>
inline void runtime_sized_copy_to(const T* src, T* dest) {
    std::memcpy(dest, src, runtime_size_of<T>());
}

template <typename T>
inline T* runtime_sized_allocate()
{
    mefdn::uint8_t* ptr = new mefdn::uint8_t[runtime_size_of<T>()];
    return reinterpret_cast<T*>(ptr);
}

template <typename T>
inline void runtime_sized_deallocate(T* ptr)
{
    delete[] reinterpret_cast<mefdn::uint8_t*>(ptr);
}

// runtime_sized_pointer
// alternative to ordinary pointers for runtime-sized objects

template <typename T>
class runtime_sized_pointer
    : public mefdn::pointer_facade<mefdn::runtime_sized_pointer, T>
{
    typedef mefdn::pointer_facade<mefdn::runtime_sized_pointer, T>   base;

public:
    operator T* () const noexcept {
        return ptr_;
    }
    
    static runtime_sized_pointer create(T* ptr) {
        runtime_sized_pointer result;
        result.ptr_ = ptr;
        return result;
    }

private:
    friend class mefdn::pointer_core_access;
    
    template <typename U>
    runtime_sized_pointer<U> cast_to() const noexcept {
        return runtime_sized_pointer<U>::create(
            reinterpret_cast<U*>(ptr_)
        );
    }
    
    void advance(std::ptrdiff_t index) noexcept {
        ptr_ += index;
    }
    
    T* ptr_;
};

template <typename T>
inline runtime_sized_pointer<T> make_runtime_sized_pointer(T* ptr) {
    return runtime_sized_pointer<T>::create(ptr);
}

} // namespace mefdn
} // namespace menps


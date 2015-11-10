
#pragma once

#include <mgbase/lang.hpp>
#include "pointer_facade.hpp"

namespace mgbase {

// runtime_sized_traits

typedef std::size_t runtime_size_t;

template <typename T>
struct runtime_sized_traits {
    static MGBASE_CONSTEXPR_FUNCTION runtime_size_t size() MGBASE_NOEXCEPT {
        return sizeof(T);
    }
};

template <typename T>
inline MGBASE_CONSTEXPR_FUNCTION runtime_size_t runtime_size_of() MGBASE_NOEXCEPT {
    return runtime_sized_traits<T>::size();
}

// Runtime-sized classes
// These classes are incomplete types.

template <runtime_size_t (*Size)()>
struct runtime_sized_struct;

template <typename T, runtime_size_t (*Size)()>
struct runtime_sized_array;

template <typename T1, typename T2>
struct runtime_sized_pair;


template <runtime_size_t (*Size)()>
struct runtime_sized_traits< runtime_sized_struct<Size> > {
    static MGBASE_CONSTEXPR_FUNCTION runtime_size_t size() MGBASE_NOEXCEPT {
        return Size();
    }
};

template <typename T, runtime_size_t (*Size)()>
struct runtime_sized_traits< runtime_sized_array<T, Size> > {
    static MGBASE_CONSTEXPR_FUNCTION runtime_size_t size() MGBASE_NOEXCEPT {
        return runtime_size_of<T>() * Size();
    }
};

template <typename T1, typename T2>
struct runtime_sized_traits< runtime_sized_pair<T1, T2> > {
    static MGBASE_CONSTEXPR_FUNCTION runtime_size_t size() MGBASE_NOEXCEPT {
        return runtime_size_of<T1>() + runtime_size_of<T2>();
    }
};

// get functions for runtime_sized_pair

template <template <typename> class Derived, typename T1, typename T2>
inline Derived<T1> get_first(const Derived< runtime_sized_pair<T1, T2> >& ptr) MGBASE_NOEXCEPT {
    return mgbase::reinterpret_pointer_cast<T1>(ptr);
}

template <template <typename> class Derived, typename T1, typename T2>
inline Derived<T2> get_second(const Derived< runtime_sized_pair<T1, T2> >& ptr) MGBASE_NOEXCEPT {
    return mgbase::reinterpret_pointer_cast<T2>(get_first(ptr) + 1);
}

template <template <typename> class Derived, typename T, runtime_size_t (*Size)()>
inline Derived<T> get_element_at(const Derived< runtime_sized_array<T, Size> >& ptr, std::size_t index) MGBASE_NOEXCEPT {
    MGBASE_ASSERT(index < Size());
    return mgbase::reinterpret_pointer_cast<T>(ptr) + index;
}

template <typename T>
inline void runtime_sized_copy_to(const T* src, T* dest) {
    std::memcpy(dest, src, runtime_size_of<T>());
}

template <typename T>
inline T* runtime_sized_allocate()
{
    mgbase::uint8_t* ptr = new mgbase::uint8_t[runtime_size_of<T>()];
    return reinterpret_cast<T*>(ptr);
}

template <typename T>
inline void runtime_sized_deallocate(T* ptr)
{
    delete[] reinterpret_cast<mgbase::uint8_t*>(ptr);
}

}


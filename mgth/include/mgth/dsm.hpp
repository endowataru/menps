
#pragma once

#include <mgbase/lang.hpp>
#include <mgbase/type_traits/enable_if.hpp>

namespace mgth {

namespace dsm {

namespace untyped {

void* allocate(mgbase::size_t alignment, mgbase::size_t size_in_bytes);

void deallocate(void*);

} // namespace untyped

template <typename T>
inline T* allocate(const mgbase::size_t size)
{
    return untyped::allocate(MGBASE_ALIGNOF(T), sizeof(T) * size);
}

template <typename T>
inline void deallocate(T* const ptr)
{
    untyped::deallocate(ptr);
}

} // namespace dsm

} // namespace mgth


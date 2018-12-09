
#pragma once

#include <menps/meomp/common.hpp>
#include <cstddef>

namespace menps {
namespace meomp {

template <typename T>
struct global_allocator
{
    using value_type = T;
    
    global_allocator() noexcept = default;
    
    template <typename U>
    global_allocator(const global_allocator<U>& /*other*/) noexcept { }
    
    MEFDN_NODISCARD
    T* allocate(const std::size_t size)
    {
        return static_cast<T*>(meomp_malloc(sizeof(T) * size));
    }
    
    void deallocate(void* const ptr, const std::size_t /*size*/)
    {
        meomp_free(ptr);
    }
};

struct global_alloc_t { };

constexpr global_alloc_t global_alloc{};

} // namespace meomp
} // namespace menps

inline void* operator new(const std::size_t size, menps::meomp::global_alloc_t /*tag*/)
{
    return meomp_malloc(size);
}
inline void operator delete(void* const ptr, menps::meomp::global_alloc_t /*tag*/)
{
    meomp_free(ptr);
}
inline void* operator new[](const std::size_t size, menps::meomp::global_alloc_t /*tag*/)
{
    return meomp_malloc(size);
}
inline void operator delete[](void* const ptr, menps::meomp::global_alloc_t /*tag*/)
{
    meomp_free(ptr);
}


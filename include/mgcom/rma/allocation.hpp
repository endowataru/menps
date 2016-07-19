
#pragma once

#include <mgcom/rma/allocator.hpp>
#include <mgcom/rma/address.hpp>
#include <mgcom/rma/pointer.hpp>

namespace mgcom {
namespace rma {

namespace untyped {

/**
 * Allocate a registered buffer from the buffer pool.
 */
inline registered_buffer allocate(allocator& alloc, const index_t size_in_bytes)
{
    return alloc.allocate(size_in_bytes);
}
inline registered_buffer allocate(const index_t size_in_bytes)
{
    return allocate(allocator::get_instance(), size_in_bytes);
}

/**
 * Deallocate a registered buffer allocated from the buffer pool.
 */
inline void deallocate(allocator& alloc, const registered_buffer& buffer)
{
    alloc.deallocate(buffer);
}
inline void deallocate(const registered_buffer& buffer)
{
    deallocate(allocator::get_instance(), buffer);
}

} // namespace untyped


/**
 * Allocate a registered buffer from the buffer pool.
 */
template <typename T>
inline local_ptr<T> allocate(allocator& alloc, std::size_t number_of_elements = 1) {
    untyped::registered_buffer buf = untyped::allocate(alloc, mgbase::runtime_size_of<T>() * number_of_elements);
    return local_ptr<T>::cast_from(untyped::to_address(buf));
}
/**
 * Allocate a registered buffer from the buffer pool.
 */
template <typename T>
inline local_ptr<T> allocate(std::size_t number_of_elements = 1) {
    return allocate<T>(allocator::get_instance(), number_of_elements);
}

template <typename T>
inline void deallocate(allocator& alloc, const local_ptr<T>& ptr)
{
    untyped::registered_buffer buf = { ptr.to_address() }; // TODO
    untyped::deallocate(alloc, buf);
}
template <typename T>
inline void deallocate(const local_ptr<T>& ptr) {
    deallocate(allocator::get_instance(), ptr);
}

} // namespace rma
} // namespace mgcom


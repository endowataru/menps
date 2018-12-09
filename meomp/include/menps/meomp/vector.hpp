
#pragma once

#include <menps/meomp/memory.hpp>
#include <vector>

namespace menps {
namespace meomp {

template <typename T>
using global_vector = std::vector<T, global_allocator<T>>;

} // namespace meomp
} // namespace menps


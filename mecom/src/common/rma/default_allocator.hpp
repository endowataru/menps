
#pragma once

#include <menps/mecom/rma/allocator.hpp>
#include <menps/mecom/rma/registrator.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace rma {

typedef mefdn::unique_ptr<allocator>   default_allocator_ptr;

default_allocator_ptr make_default_allocator(
    registrator&    reg
,   const index_t   total_size
,   const index_t   region_size
);

} // namespace rma
} // namespace mecom
} // namespace menps


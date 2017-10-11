
#pragma once

#include <mgcom/rma/allocator.hpp>
#include <mgcom/rma/registrator.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace rma {

typedef mgbase::unique_ptr<allocator>   default_allocator_ptr;

default_allocator_ptr make_default_allocator(
    registrator&    reg
,   const index_t   total_size
,   const index_t   region_size
);

} // namespace rma
} // namespace mgcom



#pragma once

#include "dist_common.hpp"
#include <mgcom/rma.hpp>

namespace mgth {

enum class global_ult_state
    : mgcom::rma::atomic_default_t
{
    ready
,   blocked
,   finished
};

struct global_ult_desc
{
    mgcom::rma::atomic_default_t lock;
    
    context_t ctx;
    
    global_ult_state state;
    
    mgbase::uint32_t detached; // Avoid using bool for FJMPI
    ult_id joiner;
    
    void* stack_ptr;
    mgbase::size_t stack_size;
    
    void* result;
};

} // namespace mgth


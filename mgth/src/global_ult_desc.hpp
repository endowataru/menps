
#pragma once

#include <mgth/common.hpp>
#include "dist_common.hpp"
#include <mgcom/rma.hpp>

namespace mgth {

typedef mgcom::rma::atomic_default_t    global_ult_state_underlying_t;

enum class global_ult_state
    : global_ult_state_underlying_t
{
    ready
,   blocked
,   finished
};

struct global_ult_desc
{
    mgcom::rma::atomic_default_t lock;
    
    global_ult_state state;
    
    mgbase::uint32_t detached; // Avoid using bool for FJMPI
    ult_id joiner;
    
    void* stack_ptr;
    mgbase::size_t stack_size;
    
    mgcom::process_id_t owner;
};

} // namespace mgth


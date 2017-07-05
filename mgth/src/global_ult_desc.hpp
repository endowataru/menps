
#pragma once

#include <mgth/common.hpp>
#include "dist_common.hpp"
#include <mgcom/rma.hpp>

namespace mgth {

//#define MGTH_ENABLE_ASYNC_WRITE_BACK

typedef mgcom::rma::atomic_default_t    global_ult_state_underlying_t;

enum class global_ult_state
    : global_ult_state_underlying_t
{
    invalid
,   ready
,   blocked
,   finished
};

typedef mgbase::int32_t global_ult_stamp_t;

struct global_ult_desc
{
    mgcom::rma::atomic_default_t lock;
    
    global_ult_state state;
    
    mgbase::uint32_t detached; // Avoid using bool for FJMPI
    ult_id joiner;
    
    void* stack_ptr;
    mgbase::size_t stack_size;
    
    mgcom::process_id_t owner;
    
    #ifdef MGTH_ENABLE_ASYNC_WRITE_BACK
    global_ult_stamp_t cur_stamp;
    global_ult_stamp_t old_stamp;
    #endif
};

} // namespace mgth


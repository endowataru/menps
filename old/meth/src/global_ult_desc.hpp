
#pragma once

#include <menps/meth/common.hpp>
#include "dist_common.hpp"
#include <menps/mecom/rma.hpp>

namespace menps {
namespace meth {

//#define METH_ENABLE_ASYNC_WRITE_BACK

typedef mecom::rma::atomic_default_t    global_ult_state_underlying_t;

enum class global_ult_state
    : global_ult_state_underlying_t
{
    invalid
,   ready
,   blocked
,   finished
};

typedef mefdn::int32_t global_ult_stamp_t;

struct global_ult_desc
{
    mecom::rma::atomic_default_t lock;
    
    global_ult_state state;
    
    mefdn::uint32_t detached; // Avoid using bool for FJMPI
    ult_id joiner;
    
    void* stack_ptr;
    mefdn::size_t stack_size;
    
    mecom::process_id_t owner;
    
    #ifdef METH_ENABLE_ASYNC_WRITE_BACK
    global_ult_stamp_t cur_stamp;
    global_ult_stamp_t old_stamp;
    #endif
};

} // namespace meth
} // namespace menps


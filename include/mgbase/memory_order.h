
#pragma once

#include <mgbase/lang.h>

// Following flags of Boost.Atomic

#ifdef MGBASE_ATOMIC_USE_STANDARD

#include <atomic>

namespace mgbase {

using std::memory_order;

using std::memory_order_relaxed;
using std::memory_order_consume;
using std::memory_order_acquire;
using std::memory_order_release;
using std::memory_order_acq_rel;
using std::memory_order_seq_cst;

} // namespace mgbase

#else

MGBASE_CPLUSPLUS_ONLY(namespace mgbase {)

enum MGBASE_C_ONLY_PREFIX(mgbase_, memory_order)
{
    MGBASE_C_ONLY_PREFIX(mgbase_, memory_order_relaxed) = 0,
    MGBASE_C_ONLY_PREFIX(mgbase_, memory_order_consume) = 1,
    MGBASE_C_ONLY_PREFIX(mgbase_, memory_order_acquire) = 2,
    MGBASE_C_ONLY_PREFIX(mgbase_, memory_order_release) = 4,
    MGBASE_C_ONLY_PREFIX(mgbase_, memory_order_acq_rel) = 6, // acquire | release
    MGBASE_C_ONLY_PREFIX(mgbase_, memory_order_seq_cst) = 14 // acq_rel | 8
};

MGBASE_CPLUSPLUS_ONLY(}) // namespace mgbase

#endif

#ifdef MGBASE_CPLUSPLUS
    typedef mgbase::memory_order    mgbase_memory_order;
    
    #define mgbase_memory_order_relaxed     mgbase::memory_order_relaxed
    #define mgbase_memory_order_consume     mgbase::memory_order_consume
    #define mgbase_memory_order_acquire     mgbase::memory_order_acquire
    #define mgbase_memory_order_release     mgbase::memory_order_release
    #define mgbase_memory_order_acq_rel     mgbase::memory_order_acq_rel
    #define mgbase_memory_order_seq_cst     mgbase::memory_order_seq_cst
#endif


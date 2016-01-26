
#pragma once

namespace mgbase {

// Following flags of Boost.Atomic

enum memory_order
{
    memory_order_relaxed = 0,
    memory_order_consume = 1,
    memory_order_acquire = 2,
    memory_order_release = 4,
    memory_order_acq_rel = 6, // acquire | release
    memory_order_seq_cst = 14 // acq_rel | 8
};

} // namespace mgbase


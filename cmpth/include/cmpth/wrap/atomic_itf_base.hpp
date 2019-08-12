
#pragma once

#include <cmpth/fdn.hpp>
#include <atomic>

namespace cmpth {

struct atomic_itf_base
{
    template <typename T>
    using atomic = std::atomic<T>;
    
    using memory_order = std::memory_order;
    
    static const memory_order memory_order_relaxed = std::memory_order_relaxed;
    static const memory_order memory_order_acquire = std::memory_order_acquire;
    static const memory_order memory_order_release = std::memory_order_release;
    static const memory_order memory_order_acq_rel = std::memory_order_acq_rel;
    static const memory_order memory_order_seq_cst = std::memory_order_seq_cst;
    // Note: memory_order_consume should not be used.
    
    static void atomic_thread_fence(memory_order order) noexcept {
        std::atomic_thread_fence(order);
    }
};

} // namespace cmpth


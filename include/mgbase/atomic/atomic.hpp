
#pragma once

#include <mgbase/lang.hpp>
#include <mgbase/atomic/atomic.h>
#include <mgbase/memory_order.hpp>
#include <mgbase/atomic/memory_barrier.hpp>

namespace mgbase {

template <typename T>
class atomic
    : noncopyable
{
public:
    atomic() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    /*
     * Note: Although C++11 standard defines this constructor as an implicit constructor,
     *       it is defined as an explicit constructor
     *       because the assignment operator in C++03 cannot be explicitly deleted.
     */
    explicit atomic(const T val) MGBASE_NOEXCEPT
        : value_(val) { }
    
    volatile atomic& operator = (const T val) volatile MGBASE_NOEXCEPT {
        this->store(val);
        return *this;
    }
    
    MGBASE_ALWAYS_INLINE T load(const memory_order order = memory_order_seq_cst) const volatile MGBASE_NOEXCEPT {
        return detail::load(&value_, order);
    }
    
    MGBASE_ALWAYS_INLINE void store(const T desired, const memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        detail::store(&value_, desired, order);
    }
    
    MGBASE_ALWAYS_INLINE T compare_exchange_weak(T& expected, const T desired,
        const memory_order success, const memory_order failure) volatile MGBASE_NOEXCEPT
    {
        return detail::compare_exchange_weak(&value_, &expected, desired, success, failure);
    }
    MGBASE_ALWAYS_INLINE T compare_exchange_weak(T& expected, const T desired,
        const memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT
    {
        return this->compare_exchange_weak(expected, desired, order, order);
    }
    
    MGBASE_ALWAYS_INLINE T compare_exchange_strong(T& expected, const T desired,
        const memory_order success, const memory_order failure) volatile MGBASE_NOEXCEPT
    {
        return detail::compare_exchange_strong(&value_, &expected, desired, success, failure);
    }
    MGBASE_ALWAYS_INLINE T compare_exchange_strong(T& expected, const T desired,
        const memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT
    {
        return this->compare_exchange_strong(expected, desired, order, order);
    }
    
    #define DEFINE_FETCH_OP(op, OP) \
        MGBASE_ALWAYS_INLINE T fetch_##op(const T arg, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT { \
            return detail::fetch_##op(&value_, arg, order); \
        }
    
    MGBASE_FETCH_OP_LIST(DEFINE_FETCH_OP)
    
    #undef DEFINE_FETCH_OP
    
private:
    volatile T value_;
};

} // namespace mgbase


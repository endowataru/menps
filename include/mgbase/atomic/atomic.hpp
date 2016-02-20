
#pragma once

#include <mgbase/lang.hpp>
#include "atomic.h"
#include <mgbase/atomic/atomic_base.hpp>
#include <mgbase/memory_order.hpp>
#include <mgbase/atomic/memory_barrier.hpp>

namespace mgbase {

// To force sequential consistency:
//#define MGBASE_ATOMIC_FORCE_SEQ_CST

template <typename T>
class atomic
    #ifdef MGBASE_CPP11_SUPPORTED
    : noncopyable
    #endif
{
    typedef typename detail::atomic_storage<T>::type    storage_type;
    
public:
    #ifdef MGBASE_CPP11_SUPPORTED
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
    #endif
    
    MGBASE_ALWAYS_INLINE T load(/*const*/ memory_order order = memory_order_seq_cst) const volatile MGBASE_NOEXCEPT
    {
        #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
            order = memory_order_seq_cst;
        #endif
        
        return detail::load<T>(&value_, order);
    }
    
    MGBASE_ALWAYS_INLINE void store(const T desired, /*const*/ memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT
    {
        #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
            order = memory_order_seq_cst;
        #endif
        
        detail::store(&value_, desired, order);
    }
    
    MGBASE_ALWAYS_INLINE bool compare_exchange_weak(T& expected, const T desired,
        /*const*/ memory_order success, /*const*/ memory_order failure) volatile MGBASE_NOEXCEPT
    {
        #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
            success = memory_order_seq_cst;
            failure = memory_order_seq_cst;
        #endif
        
        return detail::compare_exchange_weak(&value_, &expected, desired, success, failure);
    }
    MGBASE_ALWAYS_INLINE bool compare_exchange_weak(T& expected, const T desired,
        /*const*/ memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT
    {
        #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
            order = memory_order_seq_cst;
        #endif
        
        return this->compare_exchange_weak(expected, desired, order, order);
    }
    
    MGBASE_ALWAYS_INLINE bool compare_exchange_strong(T& expected, const T desired,
        /*const*/ memory_order success, /*const*/ memory_order failure) volatile MGBASE_NOEXCEPT
    {
        #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
            success = memory_order_seq_cst;
            failure = memory_order_seq_cst;
        #endif
        
        return detail::compare_exchange_strong(&value_, &expected, desired, success, failure);
    }
    MGBASE_ALWAYS_INLINE bool compare_exchange_strong(T& expected, const T desired,
        const memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT
    {
        return this->compare_exchange_strong(expected, desired, order, order);
    }
    
    #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
        #define DEFINE_FETCH_OP(name, NAME, c_op) \
        MGBASE_ALWAYS_INLINE T fetch_##name(const T arg, const memory_order /*order*/ = memory_order_seq_cst) volatile MGBASE_NOEXCEPT { \
            return detail::fetch_##name(&value_, arg, memory_order_seq_cst); \
        }
    #else
        #define DEFINE_FETCH_OP(name, NAME, c_op) \
        MGBASE_ALWAYS_INLINE T fetch_##name(const T arg, const memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT { \
            return detail::fetch_##name(&value_, arg, order); \
        }
    #endif
    MGBASE_FETCH_OP_LIST(DEFINE_FETCH_OP)
    
    #undef DEFINE_FETCH_OP
    
#ifdef MGBASE_CPP11_SUPPORTED
private:
#else
public:
#endif
    volatile storage_type value_;
};



#define MGBASE_ATOMIC(T)        mgbase::atomic<T>

#define MGBASE_ATOMIC_PTR(T)    mgbase::atomic<T*>

#define MGBASE_ATOMIC_VAR_INIT(x) { x }

} // namespace mgbase


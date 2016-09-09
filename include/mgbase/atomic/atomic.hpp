
#pragma once

#include <mgbase/lang.hpp>
#include <mgbase/memory_order.hpp>

#include "atomic.h"
#include <mgbase/atomic/atomic_base.h>

#ifndef MGBASE_ATOMIC_USE_STANDARD

#include <mgbase/atomic/atomic_base.hpp>
#include <mgbase/atomic/memory_barrier.hpp>

#include <mgbase/type_traits/sized_unsigned_integer.hpp>
#include <mgbase/force_reinterpret_cast.hpp>

namespace mgbase {

// To force sequential consistency:
//#define MGBASE_ATOMIC_FORCE_SEQ_CST

// atomic<T> should be a POD type.

template <typename T>
class atomic
{
    typedef typename sized_unsigned_integer<sizeof(T)>::type    uint_type;
    typedef typename detail::atomic_storage<uint_type>::type    storage_type;
    
public:
    #ifdef MGBASE_CXX11_SUPPORTED
    atomic() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    MGBASE_ALWAYS_INLINE
    /*implicit*/ atomic(const T val) MGBASE_NOEXCEPT
        : storage_(val) { }
    
    atomic(const atomic&) = delete;
    
    MGBASE_ALWAYS_INLINE
    volatile atomic& operator = (const T val) volatile MGBASE_NOEXCEPT {
        this->store(val);
        return *this;
    }
    
    atomic& operator = (const atomic&) = delete;
    #endif
    
    MGBASE_ALWAYS_INLINE
    T load(/*const*/ memory_order order = memory_order_seq_cst) const volatile MGBASE_NOEXCEPT
    {
        #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
            order = memory_order_seq_cst;
        #endif
        
        return force_reinterpret_cast<T>(
            detail::load<uint_type>(&storage_, order)
        );
    }
    
    MGBASE_ALWAYS_INLINE
    void store(const T desired, /*const*/ memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT
    {
        #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
            order = memory_order_seq_cst;
        #endif
        
        detail::store(
            &storage_
        ,   force_reinterpret_cast<uint_type>(desired)
        ,   order
        );
    }
    
    MGBASE_ALWAYS_INLINE
    T exchange(const T desired, /*const*/ memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT
    {
        #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
            order = memory_order_seq_cst;
        #endif
        
        return force_reinterpret_cast<T>(
            detail::exchange(
                &storage_
            ,   force_reinterpret_cast<uint_type>(desired)
            ,   order
            )
        );
    }
    
    MGBASE_ALWAYS_INLINE
    bool compare_exchange_weak(T& expected, const T desired,
        /*const*/ memory_order success, /*const*/ memory_order failure) volatile MGBASE_NOEXCEPT
    {
        #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
            success = memory_order_seq_cst;
            failure = memory_order_seq_cst;
        #endif
        
        return detail::compare_exchange_weak(
            &storage_
        ,   reinterpret_cast<uint_type*>(&expected)
        ,   force_reinterpret_cast<uint_type>(desired)
        ,   success
        ,   failure
        );
    }
    MGBASE_ALWAYS_INLINE
    bool compare_exchange_weak(T& expected, const T desired,
        /*const*/ memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT
    {
        #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
            order = memory_order_seq_cst;
        #endif
        
        return this->compare_exchange_weak(expected, desired, order, order);
    }
    
    MGBASE_ALWAYS_INLINE
    bool compare_exchange_strong(T& expected, const T desired,
        /*const*/ memory_order success, /*const*/ memory_order failure) volatile MGBASE_NOEXCEPT
    {
        #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
            success = memory_order_seq_cst;
            failure = memory_order_seq_cst;
        #endif
        
        return detail::compare_exchange_strong(
            &storage_
        ,   reinterpret_cast<uint_type*>(&expected)
        ,   force_reinterpret_cast<uint_type>(desired)
        ,   success
        ,   failure
        );
    }
    MGBASE_ALWAYS_INLINE
    bool compare_exchange_strong(T& expected, const T desired,
        const memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT
    {
        return this->compare_exchange_strong(expected, desired, order, order);
    }
    
    #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
        #define DEFINE_FETCH_OP(name, NAME, c_op) \
            MGBASE_ALWAYS_INLINE \
            T fetch_##name(const T arg, const memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT { \
                return force_reinterpret_cast<T>( \
                    detail::fetch_##name( \
                        &storage_ \
                    ,   force_reinterpret_cast<uint_type>(arg) \
                    ,   memory_order_seq_cst \
                    ) \
                ); \
            }
    #else
        #define DEFINE_FETCH_OP(name, NAME, c_op) \
            MGBASE_ALWAYS_INLINE \
            T fetch_##name(const T arg, const memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT { \
                return force_reinterpret_cast<T>( \
                    detail::fetch_##name( \
                        &storage_ \
                    ,   force_reinterpret_cast<uint_type>(arg) \
                    ,   order \
                    ) \
                ); \
            }
    #endif
    
    MGBASE_FETCH_OP_LIST(DEFINE_FETCH_OP)
    
    #undef DEFINE_FETCH_OP
    
#ifdef MGBASE_CXX11_SUPPORTED
private:
#else
public:
#endif
    // Exposed in C++03 to use aggregate initialization
    volatile storage_type storage_;
};

} // namespace mgbase

#endif



#pragma once

#error "Deprecated"

#include <mgbase/memory_order.h>

MGBASE_EXTERN_C_BEGIN

#define MGBASE_C_ATOMIC_LIST(x)  \
    x(bool                     ,   bool            )    \
    x(char                     ,   char            )    \
    x(signed char              ,   schar           )    \
    x(unsigned char            ,   uchar           )    \
    x(short                    ,   short           )    \
    x(unsigned short           ,   ushort          )    \
    x(int                      ,   int             )    \
    x(unsigned int             ,   uint            )    \
    x(long                     ,   long            )    \
    x(unsigned long            ,   ulong           )    \
    x(long long                ,   llong           )    \
    x(unsigned long long       ,   ullong          )    \
                                                        \
    x(mgbase_wchar_t           ,   wchar_t         )    \
    x(mgbase_int_least8_t      ,   int_least8_t    )    \
    x(mgbase_uint_least8_t     ,   uint_least8_t   )    \
    x(mgbase_int_least16_t     ,   int_least16_t   )    \
    x(mgbase_uint_least16_t    ,   uint_least16_t  )    \
    x(mgbase_int_least32_t     ,   int_least32_t   )    \
    x(mgbase_uint_least32_t    ,   uint_least32_t  )    \
    x(mgbase_int_least64_t     ,   int_least64_t   )    \
    x(mgbase_uint_least64_t    ,   uint_least64_t  )    \
    x(mgbase_int_fast8_t       ,   int_fast8_t     )    \
    x(mgbase_uint_fast8_t      ,   uint_fast8_t    )    \
    x(mgbase_int_fast16_t      ,   int_fast16_t    )    \
    x(mgbase_uint_fast16_t     ,   uint_fast16_t   )    \
    x(mgbase_int_fast32_t      ,   int_fast32_t    )    \
    x(mgbase_uint_fast32_t     ,   uint_fast32_t   )    \
    x(mgbase_int_fast64_t      ,   int_fast64_t    )    \
    x(mgbase_uint_fast64_t     ,   uint_fast64_t   )    \
    x(mgbase_intptr_t          ,   intptr_t        )    \
    x(mgbase_uintptr_t         ,   uintptr_t       )    \
    x(mgbase_size_t            ,   size_t          )    \
    x(mgbase_ptrdiff_t         ,   ptrdiff_t       )    \
    x(mgbase_intmax_t          ,   intmax_t        )    \
    x(mgbase_uintmax_t         ,   uintmax_t       )

#ifdef MGBASE_CPLUSPLUS
    #define DEFINE_TYPE(T, name) \
        typedef struct mgbase_atomic_##name { \
            typedef T   value_type; /*nonstandard*/ \
            volatile T val; \
        } \
        mgbase_atomic_##name;
#else
    #define DEFINE_TYPE(T, name) \
        typedef struct mgbase_atomic_##name { \
            volatile T val; \
        } \
        mgbase_atomic_##name;
#endif

MGBASE_C_ATOMIC_LIST(DEFINE_TYPE)

#undef DEFINE_TYPE

MGBASE_EXTERN_C_END

#ifdef MGBASE_CPLUSPLUS
    #include "atomic_base.hpp"
    
    namespace /*unnamed*/ {
    
    template <typename A>
    MGBASE_ALWAYS_INLINE typename A::value_type mgbase_atomic_load_explicit(
        volatile A* const           obj
    ,   const mgbase_memory_order   order
    ) MGBASE_NOEXCEPT
    {
        return mgbase::detail::load(&obj->val, order);
    }
    template <typename A>
    MGBASE_ALWAYS_INLINE typename A::value_type mgbase_atomic_load(
        volatile A* const   obj
    ) MGBASE_NOEXCEPT
    {
        return mgbase_atomic_load_explicit(obj, mgbase_memory_order_seq_cst);
    }
    
    template <typename A, typename C>
    MGBASE_ALWAYS_INLINE void mgbase_atomic_store_explicit(
        volatile A* const           obj
    ,   const C                     desired
    ,   const mgbase_memory_order   order
    ) MGBASE_NOEXCEPT
    {
        mgbase::detail::store(&obj->val, desired, order);
    }
    template <typename A, typename C>
    MGBASE_ALWAYS_INLINE void mgbase_atomic_store(
        volatile A* const   obj
    ,   const C             desired
    ) MGBASE_NOEXCEPT
    {
        mgbase_atomic_store_explicit(obj, desired, mgbase_memory_order_seq_cst);
    }
    
    template <typename A, typename C>
    MGBASE_ALWAYS_INLINE bool mgbase_atomic_compare_exchange_strong_explicit(
        volatile A* const           obj
    ,   C* const                    expected
    ,   C const                     desired
    ,   const mgbase_memory_order   success
    ,   const mgbase_memory_order   failure
    ) MGBASE_NOEXCEPT
    {
        return mgbase::detail::compare_exchange_strong(&obj->val, expected, desired, success, failure);
    }
    template <typename A, typename C>
    MGBASE_ALWAYS_INLINE bool mgbase_atomic_compare_exchange_strong(
        volatile A* const           obj
    ,   C* const                    expected
    ,   C const                     desired
    ) MGBASE_NOEXCEPT
    {
        return mgbase_atomic_compare_exchange_strong_explicit(&obj->val, expected, desired,
            mgbase_memory_order_seq_cst, mgbase_memory_order_seq_cst);
    }
    
    template <typename A, typename C>
    MGBASE_ALWAYS_INLINE bool mgbase_atomic_compare_exchange_weak_explicit(
        volatile A* const           obj
    ,   C* const                    expected
    ,   C const                     desired
    ,   const mgbase_memory_order   success
    ,   const mgbase_memory_order   failure
    ) MGBASE_NOEXCEPT
    {
        return mgbase::detail::compare_exchange_weak(&obj->val, expected, desired, success, failure);
    }
    template <typename A, typename C>
    MGBASE_ALWAYS_INLINE bool mgbase_atomic_compare_exchange_weak(
        volatile A* const           obj
    ,   C* const                    expected
    ,   C const                     desired
    ) MGBASE_NOEXCEPT
    {
        return mgbase_atomic_compare_exchange_weak_explicit(&obj->val, expected, desired,
            mgbase_memory_order_seq_cst, mgbase_memory_order_seq_cst);
    }
    
    #define DEFINE_FETCH_OP(op, OP) \
        template <typename A, typename C> \
        MGBASE_ALWAYS_INLINE C mgbase_atomic_fetch_##op##_explicit( \
            volatile A* const           obj \
        ,   C const                     arg \
        ,   const mgbase_memory_order   order \
        ) { \
            return mgbase::detail::fetch_##op(&obj->val, arg, order); \
        } \
        template <typename A, typename C> \
        MGBASE_ALWAYS_INLINE C mgbase_atomic_fetch_##op( \
            volatile A* const           obj \
        ,   C const                     arg \
        ) { \
            return mgbase_atomic_fetch_##op##_explicit(&obj->val, arg, mgbase_memory_order_seq_cst); \
        }
    
    MGBASE_FETCH_OP_LIST(DEFINE_FETCH_OP)
    
    #undef DEFINE_FETCH_OP
    
    } // unnamed namespace
    
#else
    // Please implement the standard-compliant atomic operations in C by yourself
    // if you are a C fanatic.
#endif



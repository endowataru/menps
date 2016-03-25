
#pragma once

#include <mgbase/memory_order.h>

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

// declaration of mgbase::atomic<T>

#ifdef MGBASE_ATOMIC_USE_STANDARD

    namespace mgbase {

        using std::atomic;

    } // namespace mgbase

    #define MGBASE_ATOMIC_VAR_INIT(x) { x }

#else

    namespace mgbase {

        template <typename T>
        class atomic;

    } // namespace mgbase

#endif // MGBASE_ATOMIC_USE_STANDARD

// macros for atomic types for both C and C++

#define MGBASE_ATOMIC(T)        mgbase::atomic<T>
#define MGBASE_ATOMIC_PTR(T)    mgbase::atomic<T*>

#ifdef MGBASE_CPLUSPLUS

    #define DEFINE_TYPE(T, name) \
        namespace mgbase { typedef atomic<T> atomic_##name; } \
        typedef mgbase::atomic_##name mgbase_atomic_##name;
    
    #define MGBASE_ATOMIC_VAR_INIT(x) { x }
    
#else
    #error
#endif



MGBASE_C_ATOMIC_LIST(DEFINE_TYPE)

#undef DEFINE_TYPE

#ifdef MGBASE_CPLUSPLUS
    #include "atomic.hpp"
#endif



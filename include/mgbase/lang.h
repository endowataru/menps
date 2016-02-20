
#pragma once

#include <mgbase/config.h>

// Processor Compatibility

#if (defined(__i386__) || defined(__x86_64__))
    #define MGBASE_ARCH_INTEL
    #define MGBASE_CACHE_LINE_SIZE  64
#endif

#if (defined(__sparc))
    #define MGBASE_ARCH_SPARC
    #define MGBASE_CACHE_LINE_SIZE  64
#endif

#if (defined(__sparc))
    #define MGBASE_ARCH_SPARC_V9
    
    // FIXME: How to detect SPARC64 IXfx extensions?
    #define MGBASE_ARCH_SPARC64_IXFX
#endif

// Compiler Compatibility

#ifdef __FUJITSU
    #define MGBASE_COMPILER_FUJITSU
#else
    #ifdef __GNUC__
        #define MGBASE_COMPILER_GCC
    #endif
#endif

#ifdef __clang__
    #define MGBASE_COMPILER_CLANG
#endif

// Language Compatibility

#define MGBASE_CONCAT(x, y)  MGBASE_CONCAT1(x, y)
#define MGBASE_CONCAT1(x, y) MGBASE_CONCAT2(x, y)
#define MGBASE_CONCAT2(x, y) x##y

#ifdef MGBASE_DEBUG
    #define MGBASE_ALWAYS_INLINE        inline
#else
    #define MGBASE_ALWAYS_INLINE        inline __attribute__((always_inline))
#endif

#define MGBASE_NOINLINE             __attribute__((noinline))

#define MGBASE_UNUSED               __attribute__((unused))
#define MGBASE_WARN_UNUSED_RESULT   __attribute__((warn_unused_result))

#define MGBASE_LIKELY(x)            __builtin_expect(!!(x), 1)
#define MGBASE_UNLIKELY(x)          __builtin_expect(!!(x), 0)

#ifdef MGBASE_COMPILER_FUJITSU
    #define MGBASE_UNREACHABLE()    abort()
#else
    #define MGBASE_UNREACHABLE()    __builtin_unreachable()
#endif

#ifdef MGBASE_DEBUG
    #define MGBASE_ASM_COMMENT(msg)     __asm__ __volatile__ ("# " msg)
#else
    #define MGBASE_ASM_COMMENT(msg)     
#endif

#ifndef __cplusplus
    #include <stdint.h>
    #include <stdlib.h>
    #include <stddef.h>
    
    // For C
    #if __STDC_VERSION__ < 199901L
        #error "C99 or later is only supported"
    #endif
    
    #define MGBASE_CPLUSPLUS_ONLY(x)
    #define MGBASE_C_ONLY_PREFIX(prefix, x)     prefix##x
    
    #define MGBASE_EXPLICIT_OPERATOR
    #define MGBASE_NOEXCEPT
    #define MGBASE_OVERRIDE
    #define MGBASE_NULLPTR              ((void*)0)
    #define MGBASE_EMPTY_DECL           { }
    #define MGBASE_CONSTEXPR            const
    #define MGBASE_CONSTEXPR_CPP14
    
    #define MGBASE_STATIC_ASSERT(expr) \
        MGBASE_UNUSED typedef char MGBASE_CONCAT(_static_assertion_at_line_, __LINE__)[((expr)) ? 1 : -1]
    
    #define MGBASE_STATIC_ASSERT_MSG(expr, msg) MGBASE_STATIC_ASSERT(expr)
    
    #define MGBASE_EXTERN_C_BEGIN
    #define MGBASE_EXTERN_C_END
    
    #define MGBASE_ALIGNAS(a)                   __attribute__((aligned(a)))
    #define MGBASE_DECLTYPE(x)                  __typeof__(x)
    
    #define MGBASE_NORETURN                     __attribute__((noreturn))
    
#else
    #include <stdint.h> // #include <cstdint>
    #include <cstdlib>
    #include <cstddef>
    
    #define MGBASE_CPLUSPLUS
    #define MGBASE_CPLUSPLUS_ONLY(x)    x
    #define MGBASE_C_ONLY_PREFIX(prefix, x)     x
    
    #define MGBASE_EXTERN_C_BEGIN   extern "C" {
    #define MGBASE_EXTERN_C_END     }
    
    // For C++
    #if (__cplusplus >= 201103L)
        #define MGBASE_CPP11_SUPPORTED
        #define MGBASE_IF_CPP11_SUPPORTED(t, f)     t
        
        // For C++11 or later
        #define MGBASE_EXPLICIT_OPERATOR            explicit operator
        #define MGBASE_NOEXCEPT                     noexcept
        #define MGBASE_OVERRIDE                     override
        #define MGBASE_NULLPTR                      nullptr
        #define MGBASE_STATIC_ASSERT(expr)          static_assert(expr, #expr);
        #define MGBASE_STATIC_ASSERT_MSG(expr, msg) static_assert(expr, msg);
        #define MGBASE_EMPTY_DEFINITION             = default;
        #define MGBASE_ALIGNAS(a)                   alignas(a)
        #define MGBASE_DECLTYPE(x)                  decltype(x)
        #define MGBASE_CONSTEXPR                    constexpr
        #define MGBASE_CONSTEXPR_FUNCTION           constexpr
        
        namespace mgbase {
            class noncopyable {
            public:
                noncopyable(const noncopyable&) = delete;
                noncopyable& operator = (const noncopyable) = delete;

            protected:
                noncopyable() noexcept = default;
            };
        }
        
        #define MGBASE_NORETURN                     [[noreturn]]
        
    #else
        // For C++03
        #define MGBASE_IF_CPP11_SUPPORTED(t, f)     f
        
        #define MGBASE_EXPLICIT_OPERATOR            operator
        #define MGBASE_NOEXCEPT                     throw()
        #define MGBASE_OVERRIDE
        
        namespace mgbase
        {
            class nullptr_t
            {
            public:
                template <class T>
                operator T*() const {
                    return 0;
                }

                template<class C, class T>
                operator T C::*() const
                {
                    return 0;
                }

            private:
                void operator&() const;
            };
        }
        
        #define MGBASE_NULLPTR                      (::mgbase::nullptr_t())
        
        #define MGBASE_EMPTY_DEFINITION             { }
        #define MGBASE_ALIGNAS(a)                   __attribute__((aligned(a)))
        #define MGBASE_DECLTYPE(x)                  __typeof__(x)
        #define MGBASE_CONSTEXPR                    const
        #define MGBASE_CONSTEXPR_FUNCTION           
        
        namespace mgbase {
            template <bool> struct static_assertion;
            template <> struct static_assertion<true> { };
            template <unsigned int> struct static_assert_check { };
        }
        
        #define MGBASE_STATIC_ASSERT(expr) \
            MGBASE_UNUSED typedef ::mgbase::static_assert_check<sizeof(::mgbase::static_assertion<((expr))>)> \
                MGBASE_CONCAT(_static_assertion_at_line_, __LINE__)
        
        #define MGBASE_STATIC_ASSERT_MSG(expr, msg) MGBASE_STATIC_ASSERT(expr)
        
        namespace mgbase {
            class noncopyable {
            private:
                noncopyable(const noncopyable&);
                noncopyable& operator = (const noncopyable&);

            protected:
                noncopyable() { }
            };
        }
        
        #define MGBASE_NORETURN                     __attribute__((noreturn))
    #endif

    #if (__cplusplus >= 201403L)
        // For C++14 or later
        #define MGBASE_CONSTEXPR_CPP14  constexpr
    #else
        // For C++11 or older
        #define MGBASE_CONSTEXPR_CPP14
    #endif

    namespace mgbase {
        using ::uint8_t;
        using ::int8_t;
        using ::uint16_t;
        using ::int16_t;
        using ::uint32_t;
        using ::int32_t;
        using ::uint64_t;
        using ::int64_t;
        
        using ::size_t;
        using ::ptrdiff_t;
        
        //using ::ssize_t;
        typedef int64_t  ssize_t;
        
        using ::intptr_t;
        using ::uintptr_t;
    }

#endif

typedef wchar_t         mgbase_wchar_t;

typedef uint8_t         mgbase_uint8_t;
typedef uint16_t        mgbase_uint16_t;
typedef uint32_t        mgbase_uint32_t;
typedef uint64_t        mgbase_uint64_t;

typedef int8_t          mgbase_int8_t;
typedef int16_t         mgbase_int16_t;
typedef int32_t         mgbase_int32_t;
typedef int64_t         mgbase_int64_t;

typedef int_least8_t    mgbase_int_least8_t;
typedef int_least16_t   mgbase_int_least16_t;
typedef int_least32_t   mgbase_int_least32_t;
typedef int_least64_t   mgbase_int_least64_t;
typedef uint_least8_t   mgbase_uint_least8_t;
typedef uint_least16_t  mgbase_uint_least16_t;
typedef uint_least32_t  mgbase_uint_least32_t;
typedef uint_least64_t  mgbase_uint_least64_t;

typedef int_fast8_t     mgbase_int_fast8_t;
typedef int_fast16_t    mgbase_int_fast16_t;
typedef int_fast32_t    mgbase_int_fast32_t;
typedef int_fast64_t    mgbase_int_fast64_t;
typedef uint_fast8_t    mgbase_uint_fast8_t;
typedef uint_fast16_t   mgbase_uint_fast16_t;
typedef uint_fast32_t   mgbase_uint_fast32_t;
typedef uint_fast64_t   mgbase_uint_fast64_t;

typedef size_t          mgbase_size_t;
typedef ptrdiff_t       mgbase_ptrdiff_t;
typedef intptr_t        mgbase_intptr_t;
typedef uintptr_t       mgbase_uintptr_t;

typedef intmax_t        mgbase_intmax_t;
typedef uintmax_t       mgbase_uintmax_t;



#pragma once

#include <mgbase/config.h>

// Processor Compatibility

#if (defined(__i386__) || defined(__x86_64__))
    #define MGBASE_ARCH_INTEL
    #define MGBASE_CACHE_LINE_SIZE  64
    #define MGBASE_CALL_STACK_GROW_DIR  -1
#endif

#if (defined(__sparc))
    #define MGBASE_ARCH_SPARC
    #define MGBASE_CACHE_LINE_SIZE  64
    #define MGBASE_CALL_STACK_GROW_DIR  -1
#endif

#if (defined(__sparc))
    #define MGBASE_ARCH_SPARC_V9
    
    // FIXME: How to detect SPARC64 IXfx extensions?
    #define MGBASE_ARCH_SPARC64_IXFX
#endif

// OS Compatibility

#if (defined(__linux__))
    #define MGBASE_OS_LINUX
#elif (defined(__APPLE__))
    #define MGBASE_OS_MAC_OS_X
#else
    #error "Unsupported OS"
#endif

// Compiler Compatibility

#ifdef __FUJITSU
    #define MGBASE_COMPILER_FUJITSU
#else
    // See also: https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
    
    #ifdef __GNUC__
        #define MGBASE_COMPILER_GCC
        
        #define MGBASE_COMPILER_GCC_VERSION \
            (  __GNUC__ * 10000 \
             + __GNUC_MINOR__ * 100 \
             + __GNUC_PATCHLEVEL__)
        
        #if MGBASE_COMPILER_GCC_VERSION >= 40600
            #define MGBASE_COMPILER_GCC_SUPPORTS_PRAGMA_DIAGNOSTIC
        #endif
    #endif
#endif

#ifdef __clang__
    #define MGBASE_COMPILER_CLANG
#endif

#ifdef __INTEL_COMPILER
    #define MGBASE_COMPILER_INTEL
#endif

// Language Compatibility

#define MGBASE_CONCAT(x, y)  MGBASE_CONCAT1(x, y)
#define MGBASE_CONCAT1(x, y) MGBASE_CONCAT2(x, y)
#define MGBASE_CONCAT2(x, y) x##y

#if (defined(MGBASE_COMPILER_SUPPORTS_ALWAYS_INLINE) && !defined(MGBASE_DEBUG))
    #define MGBASE_ALWAYS_INLINE    inline __attribute__((always_inline))
#else
    #define MGBASE_ALWAYS_INLINE    inline
#endif

#define MGBASE_NOINLINE             __attribute__((noinline))

#ifdef MGBASE_COMPILER_FUJITSU
    #define MGBASE_MAY_ALIAS
#else
    #define MGBASE_MAY_ALIAS        __attribute__((may_alias))
#endif

#define MGBASE_UNUSED               __attribute__((unused))
#define MGBASE_WARN_UNUSED_RESULT   __attribute__((warn_unused_result))

#define MGBASE_LIKELY(x)            __builtin_expect(!!(x), 1)
#define MGBASE_UNLIKELY(x)          __builtin_expect(!!(x), 0)

#ifdef MGBASE_COMPILER_SUPPORTS_BUILTIN_UNREACHABLE
    #define MGBASE_UNREACHABLE()    __builtin_unreachable()
#else
    #define MGBASE_UNREACHABLE()    abort()
#endif

#ifdef MGBASE_COMPILER_CLANG
    #define MGBASE_COVERED_SWITCH()
#else
    #define MGBASE_COVERED_SWITCH()     default: MGBASE_UNREACHABLE(); break;
#endif

#ifdef MGBASE_DEBUG
    #define MGBASE_ASM_COMMENT(msg)     __asm__ __volatile__ ("# " msg)
#else
    #define MGBASE_ASM_COMMENT(msg)     
#endif

#define MGBASE_GET_STACK_POINTER()      (__builtin_frame_address(0))

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
    #define MGBASE_CONSTEXPR
    #define MGBASE_CONSTEXPR_CXX14
    
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
    
    /*#ifdef MGBASE_COMPILER_INTEL
        #if defined(__GXX_EXPERIMENTAL_CPP0X__) || defined(__GXX_EXPERIMENTAL_CXX0X__)
            #define MGBASE_CXX11_SUPPORTED
        #endif
    #else*/
        #if __cplusplus >= 201103L
            #define MGBASE_CXX11_SUPPORTED
        #endif
    //#endif
    
    // For C++
    
    #ifdef MGBASE_CXX11_NOEXCEPT_SUPPORTED
        #define MGBASE_NOEXCEPT                     noexcept
        #define MGBASE_NOEXCEPT_IF(x)               noexcept(x)
    #else
        #define MGBASE_NOEXCEPT                     throw()
        #define MGBASE_NOEXCEPT_IF(x)
    #endif
    
    #ifdef MGBASE_CXX11_NULLPTR_SUPPORTED
        #define MGBASE_NULLPTR                      nullptr
        
        namespace mgbase {
        
        #ifdef MGBASE_CXX11_NULLPTR_T_SUPPORTED
            using std::nullptr_t;
        #else
            typedef decltype(nullptr)   nullptr_t;
        #endif
        
        } // namespace mgbase
    #else
        namespace mgbase
        {
            #if (defined(MGBASE_COMPILER_GCC_SUPPORTS_PRAGMA_DIAGNOSTIC) && defined(MGBASE_GCC_SUPPORTS_WZERO_AS_NULL_POINTER_CONSTANT))
                #pragma GCC diagnostic push
                #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
            #endif
            
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
            
            #if (defined(MGBASE_COMPILER_GCC_SUPPORTS_PRAGMA_DIAGNOSTIC) && defined(MGBASE_GCC_SUPPORTS_WZERO_AS_NULL_POINTER_CONSTANT))
                #pragma GCC diagnostic pop
            #endif
        }
        
        #define MGBASE_NULLPTR                      (::mgbase::nullptr_t())
    #endif
    
    
    #ifdef MGBASE_CXX11_SUPPORTED
        #define MGBASE_IF_CXX11_SUPPORTED(x)        x
        
        // For C++11 or later
        #define MGBASE_EXPLICIT_OPERATOR            explicit operator
        #define MGBASE_OVERRIDE                     override
        #define MGBASE_STATIC_ASSERT(expr)          static_assert(expr, #expr)
        #define MGBASE_STATIC_ASSERT_MSG(expr, msg) static_assert(expr, msg)
        #define MGBASE_EMPTY_DEFINITION             = default;
        #define MGBASE_ALIGNAS(a)                   alignas(a)
        #define MGBASE_DECLTYPE(x)                  decltype(x)
        #define MGBASE_CONSTEXPR                    constexpr
        
        #define MGBASE_NORETURN                     [[noreturn]]
        
        #define MGBASE_DEPRECATED                   __attribute__((deprecated))
        
    #else
        #define MGBASE_IF_CXX11_SUPPORTED(x)
        
        // For C++03
        #define MGBASE_EXPLICIT_OPERATOR            operator
        #define MGBASE_OVERRIDE
        
        
        #define MGBASE_EMPTY_DEFINITION             { }
        #define MGBASE_ALIGNAS(a)                   __attribute__((aligned(a)))
        #define MGBASE_DECLTYPE(x)                  __typeof__(x)
        #define MGBASE_CONSTEXPR                    
        
        namespace mgbase {
            template <bool> struct static_assertion;
            template <> struct static_assertion<true> { };
            template <unsigned int> struct static_assert_check { };
        }
        
        #define MGBASE_STATIC_ASSERT(expr) \
            MGBASE_UNUSED typedef ::mgbase::static_assert_check<sizeof(::mgbase::static_assertion<((expr))>)> \
                MGBASE_CONCAT(_static_assertion_at_line_, __LINE__)
        
        #define MGBASE_STATIC_ASSERT_MSG(expr, msg) MGBASE_STATIC_ASSERT(expr)
        
        #define MGBASE_NORETURN                     __attribute__((noreturn))
    #endif
    
    
    #ifdef MGBASE_CXX11_DEFAULT_FUNCTION_NOEXCEPT_SUPPORTED
        #define MGBASE_DEFAULT_NOEXCEPT     noexcept
    #else
        #define MGBASE_DEFAULT_NOEXCEPT
    #endif
    
    #ifdef MGBASE_CXX11_RANGE_BASED_FOR_SUPPORTED
        #define MGBASE_RANGE_BASED_FOR(decl, ...) \
            for (decl : __VA_ARGS__)
    #else
        // Reference:
        // http://iorate.hatenablog.com/entry/20110412/1302627774
        
        #define MGBASE_RANGE_BASED_FOR(decl, ...) \
            if (bool MGBASE_FOR_1 = true) \
            for (auto&& MGBASE_FOR_RANGE = __VA_ARGS__; MGBASE_FOR_1; MGBASE_FOR_1 = false) \
            for (auto&& MGBASE_FOR_BEGIN = MGBASE_FOR_RANGE.begin(), \
                        MGBASE_FOR_END = MGBASE_FOR_RANGE.end(); \
                MGBASE_FOR_1 && MGBASE_FOR_BEGIN != MGBASE_FOR_END; \
                MGBASE_FOR_1 && ((void)++MGBASE_FOR_BEGIN, 0)) \
                    if (!(MGBASE_FOR_1 = false)) \
                    for (decl = *MGBASE_FOR_BEGIN; !MGBASE_FOR_1; MGBASE_FOR_1=true)
    #endif
    
    #ifdef MGBASE_CXX11_THREAD_LOCAL_SUPPORTED
        #define MGBASE_THREAD_LOCAL     thread_local
    #else
        #define MGBASE_THREAD_LOCAL     __thread
    #endif
    
    #if (__cplusplus >= 201403L)
        // For C++14 or later
        #define MGBASE_CONSTEXPR_CXX14  constexpr
        
        #define MGBASE_DEPRECATED       [[deprecated]]
    #else
        // For C++11 or older
        #define MGBASE_CONSTEXPR_CXX14
        
        #define MGBASE_DEPRECATED       __attribute__((deprecated))
    #endif
    
    #ifdef  MGBASE_CXX11_ALIGNOF_SUPPORTED
        #define MGBASE_ALIGNOF(x)   alignof(x)
    #else
        #define MGBASE_ALIGNOF(x)   __alignof(x)
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
    
    #include <mgbase/noncopyable.hpp>

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



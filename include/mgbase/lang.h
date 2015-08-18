
#pragma once

// Language Compatibility

#ifndef __cplusplus
    #include <stdint.h>
    #include <stdlib.h>
    
    // For C
    #if __STDC_VERSION__ < 199901L
        #error "C99 or later is only supported"
    #endif
    
    #define MGBASE_EXPLICIT_OPERATOR
    #define MGBASE_NOEXCEPT
    #define MGBASE_OVERRIDE
    #define MGBASE_NULLPTR              0
    #define MGBASE_STATIC_ASSERT(expr, msg)
    #define MGBASE_EMPTY_DECL           { }
    #define MGBASE_CONSTEXPR_CPP14
    
    #define MGBASE_EXTERN_C_BEGIN
    #define MGBASE_EXTERN_C_END
    
#else
    #include <stdint.h> // #include <cstdint>
    #include <cstdlib>
    
    #define MGBASE_EXTERN_C_BEGIN   extern "C" {
    #define MGBASE_EXTERN_C_END     }
    
    // For C++
    #if (__cplusplus >= 201103L)
        #define MGBASE_CPP11_SUPPORTED
        
        // For C++11 or later
        #define MGBASE_EXPLICIT_OPERATOR         explicit
        #define MGBASE_NOEXCEPT                  noexcept
        #define MGBASE_OVERRIDE                  override
        #define MGBASE_NULLPTR                   nullptr
        #define MGBASE_STATIC_ASSERT(expr, msg)  static_assert(expr, msg);
        #define MGBASE_EMPTY_DEFINITION          = default;
        #define MGBASE_ALIGNAS(a)                alignas(a)
        
        namespace mgbase {
            class noncopyable {
            public:
                noncopyable(const noncopyable&) = delete;
                noncopyable& operator = (const noncopyable) = delete;

            protected:
                noncopyable() noexcept = default;
            };
        }
    #else
        // For C++03
        #define MGBASE_EXPLICIT_OPERATOR
        #define MGBASE_NOEXCEPT                  throw()
        #define MGBASE_OVERRIDE
        #define MGBASE_NULLPTR                   0
        #define MGBASE_STATIC_ASSERT(expr, msg)
        #define MGBASE_EMPTY_DEFINITION          { }
        #define MGBASE_ALIGNAS(a)                __attribute__((aligned(a)))
        
        namespace mgbase {
            class noncopyable {
            private:
                noncopyable(const noncopyable&);
                noncopyable& operator = (const noncopyable&);

            protected:
                noncopyable() { }
            };
        }
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
        //using ::ssize_t;
        typedef int64_t  ssize_t;
    }

#endif

// Processor Compatibility

#if (defined(__sparc))
    #define MGBASE_ARCH_SPARC
#endif

#if (defined(__i386__) || defined(__x86_64__))
    #define MGBASE_ARCH_INTEL
#endif


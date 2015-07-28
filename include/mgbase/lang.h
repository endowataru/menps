
#pragma once

// Language Compatibility

#ifndef __cplusplus
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
#else
    // For C++
    #if (__cplusplus >= 201103L)
        // For C++11 or later
        #define MGBASE_EXPLICIT_OPERATOR         explicit
        #define MGBASE_NOEXCEPT                  noexcept
        #define MGBASE_OVERRIDE                  override
        #define MGBASE_NULLPTR                   nullptr
        #define MGBASE_STATIC_ASSERT(expr, msg)  static_assert(expr, msg);
        #define MGBASE_EMPTY_DEFINITION          = default;
        
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

#endif

// Processor Compatibility

#if (defined(__sparc))
    #define MGBASE_ARCH_SPARC
#endif

#if (defined(__i386__) || defined(__x86_64__))
    #define MGBASE_ARCH_INTEL
#endif


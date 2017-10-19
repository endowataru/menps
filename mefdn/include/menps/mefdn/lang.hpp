
#pragma once

#include <menps/mefdn/config.h>
#include <cstdlib>
#include <cstdint>
#include <cstddef>

// Processor Compatibility

#define MEFDN_CACHE_LINE_SIZE  64

// Compiler Compatibility
    
#ifdef __GNUC__
    // See also: https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
    #define MEFDN_COMPILER_GCC
    
    #define MEFDN_COMPILER_GCC_VERSION \
        (  __GNUC__ * 10000 \
         + __GNUC_MINOR__ * 100 \
         + __GNUC_PATCHLEVEL__)
    
    #if MEFDN_COMPILER_GCC_VERSION >= 40600
        #define MEFDN_COMPILER_GCC_SUPPORTS_PRAGMA_DIAGNOSTIC
    #endif
#endif

#ifdef __clang__
    #define MEFDN_COMPILER_CLANG
#endif

#ifdef __INTEL_COMPILER
    #define MEFDN_COMPILER_INTEL
#endif

// Macros

#define MEFDN_DEFINE_DERIVED(Policy)    \
    private:\
        using derived_type = typename Policy::derived_type; \
        \
        /*constexpr*/ derived_type& derived() noexcept { \
            return static_cast<derived_type&>(*this); \
        } \
        constexpr const derived_type& derived() const noexcept { \
            return static_cast<const derived_type&>(*this); \
        }

// Standardized attributes

#define MEFDN_NORETURN      [[noreturn]]
#define MEFDN_NODISCARD     __attribute__((warn_unused_result))
#define MEFDN_MAYBE_UNUSED  __attribute__((unused))

// Non-standard built-ins

#define MEFDN_LIKELY(x)     __builtin_expect(!!(x), 1)
#define MEFDN_UNLIKELY(x)   __builtin_expect(!!(x), 0)

#ifdef MEFDN_COMPILER_SUPPORTS_BUILTIN_UNREACHABLE
    #define MEFDN_UNREACHABLE()     __builtin_unreachable()
#else
    #define MEFDN_UNREACHABLE()     abort()
#endif

#define MEFDN_VISIBILITY_DEFAULT    __attribute__((visibility("default")))
#define MEFDN_VISIBILITY_HIDDEN     __attribute__((visibility("hidden")))


namespace menps {
namespace mefdn {

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

using std::uint_least8_t;
using std::uint_least16_t;
using std::uint_least32_t;
using std::uint_least64_t;

using std::int_least8_t;
using std::int_least16_t;
using std::int_least32_t;
using std::int_least64_t;

using std::uint_fast8_t;
using std::uint_fast16_t;
using std::uint_fast32_t;
using std::uint_fast64_t;

using std::int_fast8_t;
using std::int_fast16_t;
using std::int_fast32_t;
using std::int_fast64_t;

using std::size_t;
using std::ptrdiff_t;

using std::intptr_t;
using std::uintptr_t;

} // namespace mefdn
} // namespace menps


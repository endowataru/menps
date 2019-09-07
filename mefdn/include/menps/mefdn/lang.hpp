
#pragma once

// We now depend on cmpth::fdn.
#include <cmpth/fdn.hpp>

#include <menps/mefdn/config.h>
#include <cstdlib>
#include <cstdint>
#include <cstddef>

#include <vector>

// Processor Compatibility

#define MEFDN_CACHE_LINE_SIZE       CMPTH_CACHE_LINE_SIZE
#define MEFDN_CALL_STACK_GROW_DIR   -1

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

// OS Compatibility

#if (defined(__linux__))
    #define MEFDN_OS_LINUX
#elif (defined(__APPLE__))
    #define MEFDN_OS_MAC_OS_X
#else
    #error "Unsupported OS"
#endif

// Macros

#define MEFDN_DEFINE_DERIVED(P)     CMPTH_DEFINE_DERIVED(P)

// Standard features in C++14 or later

#define MEFDN_STATIC_ASSERT(expr)           static_assert(expr, #expr)
#define MEFDN_STATIC_ASSERT_MSG(expr, msg)  static_assert(expr, msg)

#if (__cplusplus >= 201403L)
    #define MEFDN_DEPRECATED        [[deprecated]]
#else
    #define MEFDN_DEPRECATED        __attribute__((deprecated))
#endif

// Standard features which suffer from compiler bugs

#if defined(MEFDN_COMPILER_GCC) && MEFDN_COMPILER_GCC_VERSION < 50000
    // See also: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=59526
    #define MEFDN_DEFAULT_NOEXCEPT
#else
    #define MEFDN_DEFAULT_NOEXCEPT noexcept
#endif

// Standardized attributes

#define MEFDN_NODISCARD     CMPTH_NODISCARD
#define MEFDN_MAYBE_UNUSED  __attribute__((unused))

// Non-standard built-ins

#if (defined(MEFDN_COMPILER_SUPPORTS_ALWAYS_INLINE) && !defined(MEFDN_DEBUG))
    #define MEFDN_ALWAYS_INLINE     inline __attribute__((always_inline))
#else
    #define MEFDN_ALWAYS_INLINE     inline
#endif

#define MEFDN_NOINLINE      CMPTH_NOINLINE

#define MEFDN_MAY_ALIAS     __attribute__((may_alias))

#define MEFDN_LIKELY(x)     CMPTH_LIKELY(x)
#define MEFDN_UNLIKELY(x)   CMPTH_UNLIKELY(x)

#ifdef MEFDN_COMPILER_SUPPORTS_BUILTIN_UNREACHABLE
    #define MEFDN_UNREACHABLE()     __builtin_unreachable()
#else
    #define MEFDN_UNREACHABLE()     abort()
#endif

#define MEFDN_VISIBILITY_DEFAULT    __attribute__((visibility("default")))
#define MEFDN_VISIBILITY_HIDDEN     __attribute__((visibility("hidden")))

#define MEFDN_GET_STACK_POINTER()   (__builtin_frame_address(0))

// Other macros

#ifdef MEFDN_COMPILER_CLANG
    #define MEFDN_COVERED_SWITCH()
#else
    #define MEFDN_COVERED_SWITCH()  default: MEFDN_UNREACHABLE(); break;
#endif

#define MEFDN_EXTERN_C_BEGIN    extern "C" {
#define MEFDN_EXTERN_C_END      }

namespace menps {

namespace mefdn {

using namespace cmpth::fdn;

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

using std::nullptr_t;

enum class byte : unsigned char {}; // added in C++17

} // namespace mefdn

namespace fdn = mefdn;

} // namespace menps


// from Boost.Preprocessors

#define MEFDN_PP_CAT(x, y)      MEFDN_PP_CAT_AUX(x, y)
#define MEFDN_PP_CAT_AUX(x, y)  x ## y


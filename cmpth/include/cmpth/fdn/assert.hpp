
#pragma once

#include <cmpth/fdn/fdn.hpp>
#include <cstdlib>
#include <iostream> // TODO

#define CMPTH_P_ASSERT(P, x)    \
    static_cast<void>(!(P::assert_aspect_type::is_enabled) || (x) || \
    (P::assert_aspect_type::fail(#x, __FILE__, __LINE__),0))

#define CMPTH_P_ASSERT_ALWAYS(P, x)    \
    static_cast<void>((x) || (P::assert_aspect_type::fail(#x, __FILE__, __LINE__),0))

namespace cmpth {

template <bool IsEnabled>
struct assert_aspect {
    static const bool is_enabled = IsEnabled;
    static void fail(const char* const cond, const char* const file, const int line) {
        std::cerr << "Assertion failed: " << cond << " of " << file
                  << " at line " << line << "." << std::endl;
        std::abort();
    }
};

using def_assert_aspect =
    assert_aspect<
        #ifdef CMPTH_DEBUG
        true
        #else
        false
        #endif
    >;

} // namespace cmpth



#pragma once

#include <cmpth/fdn/fdn.hpp>
#include <cstdlib>

#define CMPTH_P_ASSERT(P, x)    \
    static_cast<void>(!(P::assert_policy_type::is_enabled) || (x) || \
    (P::assert_policy_type::fail(#x, __FILE__, __LINE__),0)) \

namespace cmpth {

template <bool IsEnabled>
struct assert_policy {
    static const bool is_enabled = IsEnabled;
    static void fail(const char*, const char*, int) {
        std::abort();
    }
};

using def_assert_policy = 
    assert_policy<
        #ifdef CMPTH_DEBUG
        true
        #else
        false
        #endif
    >;

} // namespace cmpth



#pragma once

#include <cmpth/fdn/fdn.hpp>
#include <cstdlib>

#define CMPTH_P_ASSERT(P, x)    \
    (void)(!(P::assert_policy_type::is_enabled) || (x) || \
    (P::assert_policy_type::fail(#x, __FILE__, __LINE__),0)) \

namespace cmpth {

template <bool IsEnabled>
struct assert_policy {
    static const bool is_enabled = IsEnabled;
    static void fail(const char*, const char*, int) {
        std::abort();
    }
};

} // namespace cmpth



#pragma once

#include <menps/mefdn/lang.hpp>
#include <sys/time.h>

namespace menps {
namespace mefdn {

inline double get_current_sec() {
    timeval t;
    gettimeofday(&t, nullptr);
    return static_cast<double>(t.tv_sec) + static_cast<double>(t.tv_usec) * 1e-6;
}

} // namespace mefdn
} // namespace menps


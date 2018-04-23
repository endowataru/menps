
#pragma once

#include <menps/mefdn/coro/coro_label.hpp>
#include <menps/mefdn/coro/coro_frame.hpp>

namespace menps {
namespace mefdn {

// This class is used at the root of a coroutine call chain.
struct identity_retcont
{
    template <typename Label>
    void operator() (Label&& /*label*/) { }
    
    template <typename Label, typename T>
    T operator() (Label&& /*label*/, T&& val) {
        return mefdn::forward<T>(val);
    }
};

} // namespace mefdn
} // namespace menps


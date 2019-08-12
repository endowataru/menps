
#pragma once

#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class rma_private_heap_alloc
{
public: 
    template <typename T, typename... Args>
    static mefdn::unique_ptr<T> make_private_uninitialized(Args&&... args) {
        return mefdn::make_unique_uninitialized<T>(mefdn::forward<Args>(args)...);
    }
};

} // namespace mecom2
} // namespace menps


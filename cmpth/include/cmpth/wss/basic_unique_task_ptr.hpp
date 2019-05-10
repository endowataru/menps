
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
struct basic_unique_task_destructor
{
    void operator() (
        typename P::task_desc_type* const tk
    ) const noexcept {
        if (tk != nullptr) {
            fdn::terminate();
        }
    }
};

template <typename P>
using basic_unique_task_ptr =
    fdn::unique_ptr<
        typename P::task_desc_type
    ,   basic_unique_task_destructor<P>
    >;

} // namespace cmpth


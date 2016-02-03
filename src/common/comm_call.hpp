
#pragma once

#include <mgbase/bound_function.hpp>

namespace mgcom {

bool try_comm_call(
    const mgbase::bound_function<void ()>&  func
,   const mgbase::operation&                on_complete
);

} // namespace mgcom


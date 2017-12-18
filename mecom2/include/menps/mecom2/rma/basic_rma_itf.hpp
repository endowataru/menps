
#pragma once

#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_rma_itf
{
public:
    using process_id_type = typename P::process_id_type;
    
    template <typename T>
    using local_ptr = typename P::template local_ptr<T>;
    
    template <typename T>
    using remote_ptr = typename P::template remote_ptr<T>;
    
    template <typename T>
    using unique_local_ptr = typename P::template unique_local_ptr<T>;
    
};

} // namespace mecom2
} // namespace menps


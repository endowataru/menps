
#pragma once

#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_rma_itf
{
public:
    using ult_itf_type = typename P::ult_itf_type;
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
    using handle_type = typename P::handle_type;
    
    template <typename T>
    using local_ptr = typename P::template local_ptr<T>;
    
    template <typename T>
    using remote_ptr = typename P::template remote_ptr<T>;
    
    template <typename T>
    using unique_local_ptr = typename P::template unique_local_ptr<T>;
};

} // namespace mecom2
} // namespace menps


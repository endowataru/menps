
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
struct basic_sct_task_desc
{
private:
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using atomic_bool_type = typename base_ult_itf_type::template atomic<bool>;
    using task_mutex_type = typename P::task_mutex_type;
    
    using continuation_type = typename P::continuation_type;
    using context_type = typename P::context_type;
    using tls_map_type = typename P::tls_map_type;
    
public:
    task_mutex_type     mtx;
    atomic_bool_type    finished;
    bool                detached;
    bool                is_root;
    continuation_type   joiner;
    void*               stk_top;
    void*               stk_bottom;
    context_type        ctx;
    tls_map_type*       tls;
};

} // namespace cmpth


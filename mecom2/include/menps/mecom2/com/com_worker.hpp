
#pragma once

#include <menps/mecom2/rma/rma_worker_state.hpp>

namespace menps {
namespace mecom2 {

struct rma_worker_state_policy
{
    using rma_sn_type = mefdn::size_t;
};

template <typename BaseWorker>
class com_worker
    : public BaseWorker
{
public: 
    using rma_state_type = rma_worker_state<rma_worker_state_policy>;
    
    rma_state_type& get_rma_state() {
        return this->rma_st_;
    }
    
private:
    rma_state_type rma_st_;
};

} // namespace mecom2
} // namespace menps


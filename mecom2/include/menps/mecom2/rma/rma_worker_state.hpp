
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/utility.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class rma_worker_state
{
public:
    using rma_sn_type = typename P::rma_sn_type;
    
    rma_sn_type generate_sn()
    {
        return ++this->latest_sn_;
    }
    
    template <typename Func>
    void flush(
        const rma_sn_type   sn
    ,   Func&&              func
    ) {
        // Only when the sequence number is too old,
        // execute the actual flush operation.
        if (sn - this->oldest_sn_ >= 0) {
            // Execute the flush.
            mefdn::forward<Func>(func)();
            
            this->oldest_sn_ = latest_sn_;
        }
    }
    
private:
    rma_sn_type oldest_sn_ = 0;
    rma_sn_type latest_sn_ = 0;
};

} // namespace mecom2
} // namespace menps


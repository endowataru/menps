
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace meult {

template <typename P>
class basic_qdlock_mutex
{
    MEFDN_DEFINE_DERIVED(P)
    
    using qdlock_core_type = typename P::qdlock_core_type;
    using qdlock_node_type = typename P::qdlock_node_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using uncond_variable_type = typename ult_itf_type::uncond_variable;
    
public:
    void lock()
    {
        auto& self = this->derived();
        auto& pool = self.get_pool();
        
        const auto cur = pool.allocate();
        
        uncond_variable_type uv;
        
        const auto prev = this->core_.start_lock(cur);
        
        if (prev != nullptr) {
            cur->uv = &uv;
            uv.wait_with(
                [this, prev, cur] {
                    this->core_.set_next(prev, cur);
                    return true;
                }
            );
        }
    }
    
    void unlock()
    {
        auto& self = this->derived();
        auto& pool = self.get_pool();
        
        const auto head = this->core_.get_head();
        
        if (this->core_.try_unlock(head)) {
            pool.deallocate(head);
            return;
        }
        
        while (true) {
            if (const auto next_head = this->core_.get_next_head(head)) {
                this->core_.follow_head(head, next_head);
                
                pool.deallocate(head);
                
                MEFDN_ASSERT(next_head->uv != nullptr);
                // Awake the next thread.
                #ifdef MEULT_QD_USE_UNCOND_ENTER_FOR_TRANSFER
                // Prefer executing the next thread immediately.
                next_head->uv->notify_enter();
                #else
                next_head->uv->notify_signal();
                #endif
                
                return;
            }
            
            #if 0
            ult_itf_type::this_thread::yield();
            #endif
        }
    }
    
protected:
    qdlock_core_type    core_;
};

} // namespace meult
} // namespace menps


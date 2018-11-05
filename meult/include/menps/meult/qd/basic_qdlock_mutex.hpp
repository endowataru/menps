
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
        
        this->core_.lock_and_wait(
            cur
        ,   [&uv] (qdlock_node_type& n) { n.uv = &uv; }
        ,   [&uv] { uv.wait(); }
        );
    }
    
    void unlock()
    {
        auto& self = this->derived();
        auto& pool = self.get_pool();
        
        if (const auto old_head = this->core_.try_unlock()) {
            pool.deallocate(old_head);
            return;
        }
        
        while (true) {
            const auto old_head = this->core_.try_follow_head();
            if (old_head != nullptr) {
                const auto next_head = this->core_.get_head();
                
                MEFDN_ASSERT(next_head->uv != nullptr);
                // Awake the next thread.
                // Prefer executing the next thread immediately.
                next_head->uv->notify_enter();
                
                pool.deallocate(old_head);
                
                return;
            }
            
            ult_itf_type::this_thread::yield();
        }
    }
    
protected:
    qdlock_core_type    core_;
};

} // namespace meult
} // namespace menps


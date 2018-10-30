
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
        ,   [&uv] { /*fmt::print("wait\n");*/ uv.wait(); }
        );
    }
    
    void unlock()
    {
        auto& self = this->derived();
        auto& pool = self.get_pool();
        
        if (const auto old_head = this->core_.try_unlock()) {
            pool.deallocate(old_head);
            //fmt::print("deallocate\n");
            return;
        }
        
        //const auto head = this->core_.get_head();
        
        while (true) {
            const auto old_head = this->core_.try_follow_head();
            if (old_head != nullptr) {
                const auto next_head = this->core_.get_head();
                
                //fmt::print("notify\n");
                MEFDN_ASSERT(next_head->uv != nullptr);
                next_head->uv->notify();
                
                pool.deallocate(old_head);
                
                return;
            }
            
            ult_itf_type::this_thread::yield();
        }
        
        #if 0
        this->tail_.spin_unlock(
            [] { ult_itf_type::this_thread::yield(); }
        );
        
        if (next != nullptr) {
            MEFDN_ASSERT(next->uv != nullptr);
            next->uv->notify();
        }
        
        pool.deallocate(cur);
        #endif
    }
    
protected:
    qdlock_core_type    core_;
};

} // namespace meult
} // namespace menps


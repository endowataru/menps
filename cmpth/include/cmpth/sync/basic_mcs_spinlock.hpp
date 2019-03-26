
#pragma once

#include <cmpth/common.hpp>

namespace cmpth {

template <typename P>
class basic_mcs_spinlock
{
    using mcs_core_type = typename P::mcs_core_type;
    using mcs_node_type = typename P::mcs_node_type;
    
    using worker_type = typename P::worker_type;
    
public:
    void lock(mcs_node_type* const cur)
    {
        const auto prev = this->core_.start_lock(cur);
        
        if (CMPTH_UNLIKELY(prev != nullptr))
        {
            cur->ready.store(false, fdn::memory_order_relaxed);
            
            mcs_core_type::set_next(prev, cur);
            
            while (!prev->ready.load(fdn::memory_order_acquire)) {
                P::cpu_relax();
            }
        }
    }
    
    void unlock(mcs_node_type* const cur)
    {
        if (CMPTH_LIKELY(this->core_.try_unlock(cur))) {
            return;
        }
        
        auto& wk = worker_type::get_cur_worker();
        
        mcs_node_type* next = nullptr;
        do {
            next = mcs_core_type::load_next(cur);
        }
        while (next == nullptr);
        
        wk.local_push_top( fdn::move(next->cont) );
    }
    
private:
    mcs_core_type core_;
};

} // namespace cmpth


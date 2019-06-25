
#pragma once

#include <menps/mectx/common.hpp>
#include <menps/mefdn/logger.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace mectx {

template <typename P>
class thread_local_worker
{
    MEFDN_DEFINE_DERIVED(P)
    
public:
    void initialize_on_this_thread()
    {
        auto& self = this->derived();
        
        MEFDN_ASSERT(cur_wk_ == nullptr);
        
        cur_wk_ = &self;
        
        MEFDN_LOG_VERBOSE(
            "msg:Starting worker.\t"
        );
        #if 0
            "rank:{}"
        ,   self.get_rank()
        #endif
    }
    
    void finalize_on_this_thread()
    {
        auto& self = this->derived();
        
        MEFDN_ASSERT(cur_wk_ != nullptr);
        
        MEFDN_LOG_VERBOSE(
            "msg:Finishing worker.\t"
        );
        #if 0
            "rank:{}\t"
        ,   self.get_rank()
        #endif
        
        cur_wk_ = nullptr;
    }
    
    static derived_type& get_current_worker()
    {
        MEFDN_ASSERT(cur_wk_ != nullptr);
        return *cur_wk_;
    }
    
    // Note: No null check in this function.
    static derived_type* get_current_worker_ptr()
    {
        return cur_wk_;
    }
    
    void check_current_worker()
    {
        MEFDN_ASSERT(this == &get_current_worker());
    }
    
private:
    static MEFDN_THREAD_LOCAL derived_type* cur_wk_;
};

template <typename P>
MEFDN_THREAD_LOCAL typename P::derived_type*
thread_local_worker<P>::cur_wk_{};

} // namespace mectx
} // namespace menps


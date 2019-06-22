
#pragma once

#include <menps/mefdn/logger.hpp>

namespace menps {
namespace meult {

template <typename Policy>
class thread_local_worker_base
{
private:
    typedef typename Policy::derived_type       derived_type;
    typedef typename Policy::ult_ref_type       ult_ref_type;

public:
    void initialize_on_this_thread()
    {
        MEFDN_ASSERT(current_worker_ == nullptr);
        
        current_worker_ = &this->derived();
        
        MEFDN_LOG_VERBOSE(
            "msg:Starting worker.\t"
            "rank:{}"
        ,   this->derived().get_rank()
        );
    }
    
    void finalize_on_this_thread()
    {
        MEFDN_ASSERT(current_worker_ != nullptr);
        
        MEFDN_LOG_VERBOSE(
            "msg:Finishing worker.\t"
            "rank:{}\t"
        ,   this->derived().get_rank()
        );
        
        current_worker_ = nullptr;
    }
    
    static derived_type& get_current_worker()
    {
        MEFDN_ASSERT(current_worker_ != nullptr);
        return *current_worker_;
    }
    
    void check_current_worker()
    {
        MEFDN_ASSERT(this == &get_current_worker());
    }
    
private:
    derived_type& derived() noexcept {
        return static_cast<derived_type&>(*this);
    }
    
    static MEFDN_THREAD_LOCAL derived_type* current_worker_;
};

template <typename Policy>
MEFDN_THREAD_LOCAL typename Policy::derived_type*
thread_local_worker_base<Policy>::current_worker_{};

} // namespace meult
} // namespace menps


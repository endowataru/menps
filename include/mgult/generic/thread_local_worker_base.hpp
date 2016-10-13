
#pragma once

#include <mgbase/logger.hpp>

namespace mgult {

template <typename Traits>
class thread_local_worker_base
{
private:
    typedef typename Traits::derived_type       derived_type;
    typedef typename Traits::ult_ref_type       ult_ref_type;

public:
    void initialize_on_this_thread()
    {
        MGBASE_ASSERT(current_worker_ == MGBASE_NULLPTR);
        
        current_worker_ = &this->derived();
        
        MGBASE_LOG_VERBOSE(
            "msg:Starting worker.\t"
            "rank:{}"
        ,   this->derived().get_rank()
        );
    }
    
    void finalize_on_this_thread()
    {
        MGBASE_ASSERT(current_worker_ != MGBASE_NULLPTR);
        
        MGBASE_LOG_VERBOSE(
            "msg:Finishing worker.\t"
            "rank:{}\t"
        ,   this->derived().get_rank()
        );
        
        current_worker_ = MGBASE_NULLPTR;
    }
    
    static derived_type& get_current_worker()
    {
        MGBASE_ASSERT(current_worker_ != MGBASE_NULLPTR);
        return *current_worker_;
    }
    
    void check_current_worker()
    {
        MGBASE_ASSERT(this == &get_current_worker());
    }
    
private:
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
    
    static MGBASE_THREAD_LOCAL derived_type* current_worker_;
};

template <typename Traits>
MGBASE_THREAD_LOCAL typename Traits::derived_type*
thread_local_worker_base<Traits>::current_worker_ = MGBASE_NULLPTR;

} // namespace mgult


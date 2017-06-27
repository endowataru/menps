
#pragma once

#include <mgbase/logger.hpp>

namespace mgult {

template <typename Policy>
class thread_specific_worker_base
{
    MGBASE_POLICY_BASED_CRTP(Policy)
    
private:
    typedef typename Policy::ult_ref_type       ult_ref_type;
    
    struct tls_policy {
        typedef derived_type    value_type;
    };
    
    typedef typename Policy::
        template thread_specific_<tls_policy>::type tls_type;
    
private:
    static tls_type& get_tls() {
        static tls_type t;
        return t;
    }

public:
    void initialize_on_this_thread()
    {
        auto& t = get_tls();
        
        MGBASE_ASSERT(t.get() == MGBASE_NULLPTR);
        
        t.set(&this->derived());
        
        MGBASE_LOG_VERBOSE(
            "msg:Starting worker.\t"
            "rank:{}"
        ,   this->derived().get_rank()
        );
    }
    
    void finalize_on_this_thread()
    {
        auto& t = get_tls();
        
        MGBASE_ASSERT(t.get() != MGBASE_NULLPTR);
        
        MGBASE_LOG_VERBOSE(
            "msg:Finishing worker.\t"
            "rank:{}\t"
        ,   this->derived().get_rank()
        );
        
        t.set(MGBASE_NULLPTR);
    }
    
    static derived_type& get_current_worker()
    {
        auto& t = get_tls();
        const auto self = t.get();
        MGBASE_ASSERT(self != MGBASE_NULLPTR);
        return *self;
    }
    
    void check_current_worker()
    {
        MGBASE_ASSERT(this == &get_current_worker());
    }
};

} // namespace mgult

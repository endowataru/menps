
#pragma once

#include <menps/mefdn/logger.hpp>

namespace menps {
namespace meult {

template <typename Policy>
class thread_specific_worker_base
{
    MEFDN_DEFINE_DERIVED(Policy)
    
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
        
        MEFDN_ASSERT(t.get() == nullptr);
        
        t.set(&this->derived());
        
        MEFDN_LOG_VERBOSE(
            "msg:Starting worker.\t"
            "rank:{}"
        ,   this->derived().get_rank()
        );
    }
    
    void finalize_on_this_thread()
    {
        auto& t = get_tls();
        
        MEFDN_ASSERT(t.get() != nullptr);
        
        MEFDN_LOG_VERBOSE(
            "msg:Finishing worker.\t"
            "rank:{}\t"
        ,   this->derived().get_rank()
        );
        
        t.set(nullptr);
    }
    
    static derived_type& get_current_worker()
    {
        auto& t = get_tls();
        const auto self = t.get();
        MEFDN_ASSERT(self != nullptr);
        return *self;
    }
    
    void check_current_worker()
    {
        MEFDN_ASSERT(this == &get_current_worker());
    }
};

} // namespace meult
} // namespace menps


#pragma once

#include <menps/mefdn/logger.hpp>

namespace menps {
namespace mectx {

template <typename P>
class thread_specific_worker
{
private:
    MEFDN_DEFINE_DERIVED(P)
    
    using base_ult_itf_type = typename P::base_ult_itf_type;
    
    struct tss_policy {
        using value_type = derived_type;
    };
    
    using tss_type =
        typename base_ult_itf_type::
            template thread_specific<tss_policy>;
    
    static tss_type& get_tss() {
        static tss_type t;
        return t;
    }

public:
    void initialize_on_this_thread()
    {
        auto& t = get_tss();
        MEFDN_ASSERT(t.get() == nullptr);
        
        t.set(&this->derived());
        
        MEFDN_LOG_VERBOSE(
            "msg:Starting worker.\t"
            /*"rank:{}"
        ,   this->derived().get_rank()*/
        );
    }
    
    void finalize_on_this_thread()
    {
        auto& t = get_tss();
        MEFDN_ASSERT(t.get() != nullptr);
        
        MEFDN_LOG_VERBOSE(
            "msg:Finishing worker.\t"
            /*"rank:{}\t"
        ,   this->derived().get_rank()*/
        );
        
        t.set(nullptr);
    }
    
    static derived_type& get_current_worker()
    {
        auto& t = get_tss();
        const auto self = t.get();
        MEFDN_ASSERT(self != nullptr);
        return *self;
    }
    
    // Note: This member doesn't do the null check.
    static derived_type* get_current_worker_ptr()
    {
        auto& t = get_tss();
        return t.get();
    }
    
    void check_current_worker()
    {
        MEFDN_ASSERT(this == &get_current_worker());
    }
};

} // namespace mectx
} // namespace menps


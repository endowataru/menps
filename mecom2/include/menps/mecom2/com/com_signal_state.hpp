
#pragma once

#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

class com_signal_state
{
    //using ult_itf_type = medev2::defalt_ult_itf;
    
    // We are only interested in the state of the current "kernel" thread.
    using ult_itf_type = meult::klt_policy;
    
    struct tss_policy {
        using value_type = void;
    };
    
    using tss_type =
        typename ult_itf_type::
            template thread_specific<tss_policy>;
    
    static tss_type& get_tss() {
        static tss_type t;
        return t;
    }
    
public:
    static void set_entering_signal() {
        auto& t = get_tss();
        MEFDN_ASSERT(t.get() == nullptr);
        
        t.set(&t);
    }
    
    static void set_exiting_signal() {
        auto& t = get_tss();
        MEFDN_ASSERT(t.get() == &t);
        
        t.set(nullptr);
    }
    
    static bool is_in_signal() {
        auto& t = get_tss();
        return t.get() != nullptr;
    }
};

} // namespace mecom2
} // namespace menps


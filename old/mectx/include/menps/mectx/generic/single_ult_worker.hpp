
#pragma once

#include <menps/mectx/generic/ult_switcher.hpp>

namespace menps {
namespace mectx {

template <typename P>
class single_ult_worker
    : public ult_switcher<P> // XXX
{
    MEFDN_DEFINE_DERIVED(P)
    
    using ult_ref_type = typename P::ult_ref_type;
    using transfer_type = typename P::transfer_type;
    
public:
    template <typename Func>
    void execute(Func func)
    {
        auto& self = this->derived();
        
        self.suspend_to_new(
            self.get_root_ult()
        ,   self.get_work_ult()
        ,   [&func] (derived_type& /*self2*/, ult_ref_type& /*prev_th*/, ult_ref_type& /*next_th*/) {
                func();
                
                MEFDN_UNREACHABLE();
            }
        );
    }
    
    void suspend()
    {
        auto& self = this->derived();
        
        self.suspend_to_cont(
            self.get_work_ult()
        ,   self.get_root_ult()
        ,   [] (derived_type& self2, ult_ref_type& /*prev_th*/, ult_ref_type& /*next_th*/) {
                return transfer_type{ &self2 }; // TODO
            }
        );
    }
    
    void resume()
    {
        auto& self = this->derived();
        
        self.suspend_to_cont(
            self.get_root_ult()
        ,   self.get_work_ult()
        ,   [] (derived_type& self2, ult_ref_type& /*prev_th*/, ult_ref_type& /*next_th*/) {
                return transfer_type{ &self2 }; // TODO
            }
        );
    }
    
    void exit()
    {
        auto& self = this->derived();
        
        self.exit_to_cont(
            self.get_work_ult()
        ,   self.get_root_ult()
        ,   [] (derived_type& self2, ult_ref_type& /*prev_th*/, ult_ref_type& /*next_th*/) {
                return transfer_type{ &self2 }; // TODO
            }
        );
    }
};

} // namespace mectx
} // namespace menps


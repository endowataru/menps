
#pragma once

#include <menps/meomp/common.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace meomp {

template <typename P>
class child_worker
{
    MEFDN_DEFINE_DERIVED(P)
    
    #ifndef MEOMP_USE_CMPTH
    using ult_ref_type = typename P::ult_ref_type;
    #endif
    using worker_group_type = typename P::worker_group_type;
    
    using cmd_info_type = typename P::cmd_info_type;
    using cmd_code_type = typename P::cmd_code_type;
    
    using omp_func_type = void (*)(void*);
    using omp_data_type = void*;
    
public:
    void loop(
        worker_group_type&  wg
    ,   const omp_func_type func
    ,   const omp_data_type data
    ) {
        auto& self = this->derived();
        
        self.start_worker(
        #ifndef MEOMP_USE_CMPTH
            ult_ref_type::make_root()
        ,   self.make_work_ult()
        ,   
        #endif
            [func, data, &wg, this] {
                auto& self2 = this->derived();
                auto& sp = wg.get_dsm_space();
                
                sp.enable_on_this_thread();
                
                MEFDN_LOG_DEBUG(
                    "msg:Entering parallel region on child worker.\t"
                    "func:0x{:x}\t"
                    "data:0x{:x}"
                ,   reinterpret_cast<mefdn::uintptr_t>(func)
                ,   reinterpret_cast<mefdn::uintptr_t>(data)
                );
                
                // Enter the code block inside the parallel region.
                func(data);
                
                sp.disable_on_this_thread();
                
                // Exit the parallel region in the child worker.
                // The context of the child worker is abandoned.
                self2.exit_parallel();
                
                MEFDN_UNREACHABLE();
            }
        );
        
        while (true)
        {
            auto cmd = self.wait_for_cmd();
            
            if (!this->execute_command(wg, cmd)) {
                break;
            }
            
            self.reset_cmd();
        }
        
        self.end_worker();
    }
    
private:
    bool execute_command(
        worker_group_type&      wg
    ,   const cmd_info_type&    cmd
    ) {
        auto& self = this->derived();
        
        switch (cmd.code) {
            case cmd_code_type::barrier: {
                MEFDN_LOG_VERBOSE("msg:Start barrier on child worker.");
                wg.barrier_on(self);
                MEFDN_LOG_VERBOSE("msg:Exit barrier on child worker.");
                break;
            }
            
            case cmd_code_type::exit_parallel:
                // Exit this function.
                MEFDN_LOG_VERBOSE("msg:Exiting parallel region on child worker.");
                return false;
            
            #ifdef MEOMP_SEPARATE_WORKER_THREAD
            case cmd_code_type::try_upgrade: {
                if (!wg.get_dsm_space().try_upgrade(cmd.data)) {
                    abort(); // TODO
                }
                break;
            }
            #endif
            
            case cmd_code_type::none:
            case cmd_code_type::start_parallel:
            case cmd_code_type::end_parallel:
            case cmd_code_type::exit_program:
            default:
                // Fatal error.
                P::fatal_error();
                MEFDN_UNREACHABLE();
        }
        
        return true;
    }
};

} // namespace meomp
} // namespace menps



#pragma once

#include <menps/meomp/common.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace meomp {

template <typename P>
class child_worker
{
    MEFDN_DEFINE_DERIVED(P)
    
    using ult_ref_type = typename P::ult_ref_type;
    using worker_group_type = typename P::worker_group_type;
    
    using cmd_info_type = typename P::cmd_info_type;
    using cmd_code_type = typename P::cmd_code_type;
    #if 0
    enum class cmd_code {
        barrier = 0
    ,   exit_parallel
    };
    #endif
    
    using omp_func_type = void (*)(void*);
    using omp_data_type = void*;
    
public:
    void loop(
        worker_group_type&  wg
    ,   const omp_func_type func
    ,   const omp_data_type data
    ) {
        auto& self = this->derived();
        
        self.initialize_on_this_thread();
        
        self.execute(
            [func, data, this] {
                auto& self2 = this->derived();
                
                MEFDN_LOG_DEBUG(
                    "msg:Entering parallel region on child worker.\t"
                    "func:0x{:x}\t"
                    "data:0x{:x}"
                ,   reinterpret_cast<mefdn::uintptr_t>(func)
                ,   reinterpret_cast<mefdn::uintptr_t>(data)
                );
                
                // Enter the code block inside the parallel region.
                func(data);
                
                self2.exit_parallel();
                #if 0
                self2.code_ = cmd_code::exit_parallel;
                
                // Exit the parallel region in this worker.
                self2.exit();
                #endif
                
                MEFDN_UNREACHABLE();
            }
        );
        
        while (true)
        {
            const auto& cmd = self.get_cmd_info();
            
            if (!this->execute_command(wg, cmd)) {
                break;
            }
            
            self.reset_cmd_info();
            
            self.resume();
        }
        
        #if 0
        while (true)
        {
            switch (self.code_) {
                case cmd_code::barrier: {
                    wg.barrier_on(self);
                    break;
                }
                
                case cmd_code::exit_parallel:
                    // Exit this function.
                    return;
            }
            
            self.resume();
        }
        #endif
        
        self.finalize_on_this_thread();
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
            
            default:
                // Fatal error.
                P::fatal_error();
                MEFDN_UNREACHABLE();
        }
        
        return true;
    }
    
    #if 0
    void barrier()
    {
        auto& self = this->derived();
        
        self.code_ = cmd_code::barrier;
        
        self.suspend();
    }
    
private:
    cmd_code code_ = cmd_code::barrier;
    #endif
};

} // namespace meomp
} // namespace menps


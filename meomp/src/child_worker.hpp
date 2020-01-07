
#pragma once

#include <menps/meomp/common.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace meomp {

template <typename P>
class child_worker
{
    MEFDN_DEFINE_DERIVED(P)
    
    using worker_base_type = typename P::worker_base_type;
    
    using worker_group_type = typename P::worker_group_type;
    
    using cmd_info_type = typename P::cmd_info_type;
    using cmd_code_type = typename P::cmd_code_type;
    
    using omp_func_type = void (*)(void*);
    using omp_data_type = void*;

    using prof_aspect_type = typename P::prof_aspect_type;
    using prof_kind_type = typename prof_aspect_type::kind_type;
    template <prof_kind_type Kind>
    using prof_record_t = typename prof_aspect_type::template record_t<Kind>;
    
public:
    void loop(
        worker_group_type&  wg
    ,   const omp_func_type func
    ,   const omp_data_type data
    ) {
        CMPTH_P_PROF_SCOPE(P, meomp_parallel_child);
        
        auto& self = this->derived();
        
        on_loop_data d{ wg, func, data };
        self.template start_worker<on_loop>(&d);
        
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
    struct on_loop_data
    {
        worker_group_type&  wg;
        omp_func_type       func;
        omp_data_type       data;
    };
    
    struct on_loop
    {
        void operator() (
            worker_base_type&   self_base
        ,   on_loop_data* const pd
        ) const
        {
            auto& self = static_cast<derived_type&>(self_base);
            
            // Copy on_loop_data from the original thread's stack.
            const auto d = *pd;
            
            auto& sp = d.wg.get_dsm_space();
            sp.enable_on_this_thread();
            
            MEFDN_LOG_DEBUG(
                "msg:Entering parallel region on child worker.\t"
                "func:0x{:x}\t"
                "data:0x{:x}"
            ,   reinterpret_cast<mefdn::uintptr_t>(d.func)
            ,   reinterpret_cast<mefdn::uintptr_t>(d.data)
            );
            
            // Enter the code block inside the parallel region.
            d.func(d.data);
            
            sp.disable_on_this_thread();
            
            // Exit the parallel region in the child worker.
            // The context of the child worker is abandoned.
            self.exit_parallel();
            
            MEFDN_UNREACHABLE();
        }
    };
    
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

            case cmd_code_type::prof_begin: {
                this->prof_rec_main_comp_ =
                    prof_aspect_type::template begin_event<prof_kind_type::meomp_main_comp>();
                break;
            }
            case cmd_code_type::prof_end: {
                prof_aspect_type::template end_event<
                    prof_kind_type::meomp_main_comp>(this->prof_rec_main_comp_);
                break;
            }
            
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

    prof_record_t<prof_kind_type::meomp_main_comp> prof_rec_main_comp_;
};

} // namespace meomp
} // namespace menps


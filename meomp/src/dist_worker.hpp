
#pragma once

#include <menps/meomp/common.hpp>
#include <menps/mectx/generic/single_ult_worker.hpp>

namespace menps {
namespace meomp {

template <typename P>
class dist_worker
{
    MEFDN_DEFINE_DERIVED(P)
    
    using cmd_info_type = typename P::cmd_info_type;
    using cmd_code_type = typename P::cmd_code_type;
    
    using ult_ref_type = typename P::ult_ref_type;
    
public:
    void execute_loop()
    {
        auto& self = this->derived();
        auto& coll = self.get_comm_coll();
        
        if (coll.this_proc_id() == 0) {
            self.loop_master();
        }
        else {
            self.loop_slave();
        }
    }
    
private:
    void loop_master()
    {
        auto& self = this->derived();
        
        self.start_worker(
            ult_ref_type::make_root()
        ,   self.make_work_ult()
        ,   [this] {
                auto& self2 = this->derived();
                auto& sp = self2.get_dsm_space();
                
                sp.enable_on_this_thread();
                
                // Enter the user-defined main function.
                self2.call_entrypoint();
                
                sp.disable_on_this_thread();
                
                // Exit the program.
                // This context is also abandoned.
                self2.exit_program();
                
                MEFDN_UNREACHABLE();
            }
        );
        
        while (true)
        {
            auto& coll = self.get_comm_coll();
            auto& sp = self.get_dsm_space();
            
            auto cmd = self.wait_for_cmd();
            
            // If the master thread is in the parallel region,
            // it should not broadcast messages for issuing barriers.
            if (!is_parallel_) {
                // Broadcast the command from this process.
                coll.broadcast(0, &cmd, 1);
                
                // Synchronize the whole DSM space.
                sp.barrier();
            }
            
            if (!this->execute_command(cmd, true)) {
                break;
            }
            
            self.reset_cmd();
        }
        
        self.end_worker();
    }
    
    void loop_slave()
    {
        auto& self = this->derived();
        
        while (true)
        {
            auto& coll = self.get_comm_coll();
            auto& sp = self.get_dsm_space();
            
            cmd_info_type cmd = cmd_info_type();
        
            // Load the function pointer and the data from the master process.
            coll.broadcast(0, &cmd, 1);
            
            // Synchronize the whole DSM space.
            sp.barrier();
            
            if (!this->execute_command(cmd, false)) {
                break;
            }
        }
    }
    
private:
    bool execute_command(const cmd_info_type& cmd, bool is_master)
    {
        auto& self = this->derived();
        auto& coll = self.get_comm_coll();
        
        switch (cmd.code) {
            case cmd_code_type::start_parallel: {
                const auto proc_id  = coll.this_proc_id();
                const auto num_procs = coll.get_num_procs();
                
                // Create a worker group.
                const int threads_per_proc = get_threads_per_proc();
                const int total_num_threads = num_procs * threads_per_proc;
                
                const int thread_num_first = proc_id * threads_per_proc;
                const int num_threads = threads_per_proc;
                
                self.start_parallel_on_children(cmd.func, cmd.data,
                    total_num_threads, thread_num_first, num_threads, is_master);
                
                this->is_parallel_ = true;
                
                if (is_master) {
                    self.set_thread_num(0);
                    self.set_num_threads(total_num_threads);
                }
                else {
                    // There is no master thread on slave nodes.
                    // In this implementation, the master node does not broadcast the message
                    // to finish the parallel region
                    // because such synchronization is already done in the child threads.
                    // For there reasons, the parallel region is immediately joined here.
                    
                    self.end_parallel_on_children();
                    
                    auto& sp = self.get_dsm_space();
                    // Insert a DSM barrier here because of a observed bug.
                    // TODO: Refactoring.
                    sp.barrier();
                    
                    this->is_parallel_ = false;
                }
                
                break;
            }
            
            case cmd_code_type::end_parallel: {
                if (is_master) {
                    self.end_parallel_on_children();
                    
                    auto& sp = self.get_dsm_space();
                    // Insert a DSM barrier here because of a observed bug.
                    // TODO: Refactoring.
                    sp.barrier();
                    
                    self.set_thread_num(0);
                    self.set_num_threads(1);
                    
                    this->is_parallel_ = false;
                }
                else {
                    // Fatal error.
                    P::fatal_error();
                    MEFDN_UNREACHABLE();
                }
                
                break;
            }
            
            case cmd_code_type::barrier: {
                self.barrier_on_master();
                break;
            }
            
            case cmd_code_type::exit_program:
                // Exit this function.
                return false;
            
            #ifdef MEOMP_SEPARATE_WORKER_THREAD
            case cmd_code_type::try_upgrade: {
                if (!self.get_dsm_space().try_upgrade(cmd.data)) {
                    abort(); // TODO
                }
                break;
            }
            #endif
            
            case cmd_code_type::none:
            case cmd_code_type::exit_parallel:
            default:
                // Fatal error.
                P::fatal_error();
                MEFDN_UNREACHABLE();
        }
        
        return true;
    }
    
public:
    static int get_threads_per_proc() {
        static const auto ret = get_threads_per_proc_nocache();
        return ret;
    }
    
    static int get_threads_per_proc_nocache() {
        if (const auto str = std::getenv("MEOMP_NUM_THREADS")) {
            const auto ret = std::atoi(str);
            MEFDN_ASSERT(ret > 0);
            return ret;
        }
        else
            return 1;
    }
    
private:
    bool is_parallel_ = false;
};

} // namespace meomp
} // namespace menps



#pragma once

#include <menps/meomp/common.hpp>

namespace menps {
namespace meomp {

template <typename P>
class dist_worker
{
    MEFDN_DEFINE_DERIVED(P)
    
    using worker_base_type = typename P::worker_base_type;
    
    using cmd_info_type = typename P::cmd_info_type;
    using cmd_code_type = typename P::cmd_code_type;

    using prof_aspect_type = typename P::prof_aspect_type;
    using prof_kind_type = typename prof_aspect_type::kind_type;
    template <prof_kind_type Kind>
    using prof_record_t = typename prof_aspect_type::template record_t<Kind>;
    
public:
    dist_worker() {
        worker_base_type::initialize_tls_global();
    }
    ~dist_worker() {
        worker_base_type::finalize_tls_global();
    }
    
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
        
        self.template start_worker<on_loop>();
        
        while (true)
        {
            auto& coll = self.get_comm_coll();
            auto cmd = self.wait_for_cmd();
            
            // If the master thread is in the parallel region,
            // it should not broadcast messages for issuing barriers.
            if (!is_parallel_) {
                // Broadcast the command from this process.
                coll.broadcast(0, &cmd, 1);
            }
            
            if (!this->execute_command(cmd, true)) {
                break;
            }
            
            self.reset_cmd();
        }
        
        self.end_worker();
    }
    
private:
    struct on_loop
    {
        void operator() (worker_base_type& self_base)
        {
            auto& self = static_cast<derived_type&>(self_base);
            
            auto& sp = self.get_dsm_space();
            
            sp.enable_on_this_thread();
            
            // Enter the user-defined main function.
            self.call_entrypoint();
            
            sp.disable_on_this_thread();
            
            // Exit the program.
            // This context is also abandoned.
            self.exit_program();
            
            MEFDN_UNREACHABLE();
        }
    };
    
public:
    void loop_slave()
    {
        auto& self = this->derived();
        
        while (true)
        {
            auto& coll = self.get_comm_coll();
            cmd_info_type cmd = cmd_info_type();
            
            // Load the function pointer and the data from the master process.
            coll.broadcast(0, &cmd, 1);
            
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
                this->prof_rec_parallel_ =
                    prof_aspect_type::template begin_event<prof_kind_type::meomp_parallel>();

                auto& sp = self.get_dsm_space();
                // Synchronize the whole DSM space.
                sp.barrier();

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

                    this->prof_rec_parallel_master_ =
                        prof_aspect_type::template begin_event<prof_kind_type::meomp_parallel_master>();
                }
                else {
                    // There is no master thread on slave nodes.
                    // In this implementation, the master node does not broadcast the message
                    // to finish the parallel region
                    // because such synchronization is already done in the child threads.
                    // For there reasons, the parallel region is immediately joined here.
                    
                    self.end_parallel_on_children();
                    
                    // Insert a DSM barrier here because of a observed bug.
                    // TODO: Refactoring.
                    sp.barrier();
                    
                    this->is_parallel_ = false;

                    prof_aspect_type::template end_event<
                        prof_kind_type::meomp_parallel>(this->prof_rec_parallel_);
                }
                
                break;
            }
            
            case cmd_code_type::end_parallel: {
                if (is_master) {
                    prof_aspect_type::template end_event<
                        prof_kind_type::meomp_parallel_master>(this->prof_rec_parallel_master_);

                    self.end_parallel_on_children();
                    
                    auto& sp = self.get_dsm_space();
                    // Insert a DSM barrier here because of a observed bug.
                    // TODO: Refactoring.
                    sp.barrier();
                    
                    self.set_thread_num(0);
                    self.set_num_threads(1);
                    
                    this->is_parallel_ = false;

                    prof_aspect_type::template end_event<
                        prof_kind_type::meomp_parallel>(this->prof_rec_parallel_);
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
            
            case cmd_code_type::exit_program:
                // Exit this function.
                return false;
            
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
    prof_record_t<prof_kind_type::meomp_main_comp> prof_rec_main_comp_;
    prof_record_t<prof_kind_type::meomp_parallel> prof_rec_parallel_;
    prof_record_t<prof_kind_type::meomp_parallel_master> prof_rec_parallel_master_;
};

} // namespace meomp
} // namespace menps


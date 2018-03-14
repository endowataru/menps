
#pragma once

#include <menps/meomp/common.hpp>

namespace menps {
namespace meomp {

template <typename P>
class omp_worker_base
{
    MEFDN_DEFINE_DERIVED(P)
    
    using cmd_info_type = typename P::cmd_info_type;
    using cmd_code_type = typename P::cmd_code_type;
    
    using omp_func_type = void (*)(void*);
    using omp_data_type = void*;
    
public:
    int get_thread_num() const noexcept {
        return this->thread_num_;
    }
    int get_num_threads() const noexcept {
        return this->num_threads_;
    }

    void set_thread_num(const int num) noexcept {
        this->thread_num_ = num;
    }
    void set_num_threads(const int num) noexcept {
        this->num_threads_ = num;
    }
    
    // Called on #pragma omp barrier.
    void barrier()
    {
        auto& self = this->derived();
        
        cmd_info_type info{};
        info.code = cmd_code_type::barrier;
        this->info_ = info;
        
        self.suspend();
    }
    
    // Called at the start of #pragma omp parallel.
    void start_parallel(
        const omp_func_type func
    ,   const omp_data_type data
    ) {
        auto& self = this->derived();
        
        cmd_info_type info{};
        info.code = cmd_code_type::start_parallel;
        info.func = func;
        info.data = data;
        this->info_ = info;
        
        self.suspend();
    }
    
    // Called at the end of #pragma omp parallel.
    void end_parallel()
    {
        auto& self = this->derived();
        
        cmd_info_type info{};
        info.code = cmd_code_type::end_parallel;
        this->info_ = info;
        
        self.suspend();
    }
    
    void exit_parallel()
    {
        auto& self = this->derived();
        
        cmd_info_type info{};
        info.code = cmd_code_type::exit_parallel;
        this->info_ = info;
        
        self.exit();
    }
    
    void exit_program()
    {
        auto& self = this->derived();
        
        cmd_info_type info{};
        info.code = cmd_code_type::exit_program;
        this->info_ = info;
        
        self.exit();
    }
    
    cmd_info_type get_cmd_info() const noexcept {
        return this->info_;
    }
    void reset_cmd_info() noexcept {
        this->info_ = cmd_info_type{};
    }
    
private:
    int thread_num_ = 0;
    int num_threads_ = 1;
    cmd_info_type info_{};
};

} // namespace meomp
} // namespace menps


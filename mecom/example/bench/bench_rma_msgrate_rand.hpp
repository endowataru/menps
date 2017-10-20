
#pragma once

#include "bench_rma_msgrate.hpp"
#include <cstdlib>

template <typename 
class bench_rma_msgrate_rand
    : public bench_rma_msgrate
{
private:
    virtual mecom::process_id_t select_target_proc()
    {
        /*if (mecom::number_of_processes() == 1)
            return 0;
        
        const auto p = static_cast<mecom::process_id_t>(
            std::rand() / (RAND_MAX + 1e-5) * (mecom::number_of_processes() - 1)
        );
        
        
        const auto proc = p == mecom::current_process_id() ? (mecom::number_of_processes() - 1) : p;
        
        MEFDN_ASSERT(proc < mecom::number_of_processes());
        
        return proc;*/
        
        const auto p = static_cast<mecom::process_id_t>(
            std::rand() / (RAND_MAX + 1e-5) * (mecom::number_of_processes() - 1)
        );
        
        const auto target_proc = (mecom::current_process_id() + p + 1) % mecom::number_of_processes();
        
        return target_proc;
    }
};


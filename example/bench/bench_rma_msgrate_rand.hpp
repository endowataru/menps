
#pragma once

#include "bench_rma_msgrate.hpp"
#include <cstdlib>

class bench_rma_msgrate_rand
    : public bench_rma_msgrate
{
private:
    virtual mgcom::process_id_t select_target_proc()
    {
        /*if (mgcom::number_of_processes() == 1)
            return 0;
        
        const auto p = static_cast<mgcom::process_id_t>(
            std::rand() / (RAND_MAX + 1e-5) * (mgcom::number_of_processes() - 1)
        );
        
        
        const auto proc = p == mgcom::current_process_id() ? (mgcom::number_of_processes() - 1) : p;
        
        MGBASE_ASSERT(proc < mgcom::number_of_processes());
        
        return proc;*/
        
        const auto p = static_cast<mgcom::process_id_t>(
            std::rand() / (RAND_MAX + 1e-5) * (mgcom::number_of_processes() - 1)
        );
        
        const auto target_proc = (mgcom::current_process_id() + p + 1) % mgcom::number_of_processes();
        
        return target_proc;
    }
};


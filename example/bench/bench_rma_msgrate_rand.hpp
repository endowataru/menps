
#pragma once

#include "bench_rma_msgrate.hpp"
#include <cstdlib>

class bench_rma_msgrate_rand
    : public bench_rma_msgrate
{
private:
    virtual mgcom::process_id_t select_target_proc() {
        return std::rand() % mgcom::number_of_processes();
    }
};


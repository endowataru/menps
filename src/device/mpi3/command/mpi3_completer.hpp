
#pragma once

#include "device/mpi/command/mpi_completer.hpp"
#include "device/mpi3/mpi3_error.hpp"
#include "device/mpi3/rma.hpp"
#include <mgbase/container/circular_buffer.hpp>

namespace mgcom {
namespace mpi3 {

class mpi3_completer
{
    static const index_t max_completion_count = 16;
    
public:
    void initialize()
    {
        mpi1_.initialize();
        
        completions_.set_capacity(max_completion_count);
    }
    
    void finalize()
    {
        mpi1_.finalize();
    }
    
    mpi::mpi_completer& get_mpi1_completer() {
        return mpi1_;
    }
    
    bool full() {
        return completions_.full();
    }
    
    void add_completion(const mgbase::operation& on_complete)
    {
        completions_.push_back(on_complete);
    }
    
    void poll_on_this_thread()
    {
        mpi1_.poll_on_this_thread();
        
        flush();
    }

private:
    void flush()
    {
        if (completions_.empty())
            return;
        
        mpi3_error::check(
            MPI_Win_flush_all(rma::get_win())
        );
        
        while (!completions_.empty())
        {
            const mgbase::operation op = completions_.front();
            completions_.pop_front();
            
            mgbase::execute(op);
        }
    }
    
    mpi::mpi_completer mpi1_;
    
    mgbase::circular_buffer<mgbase::operation> completions_;
};

} // namespace mpi3
} // namespace mgcom


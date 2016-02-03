
#pragma once

#include "device/mpi/command/mpi_completer.hpp"
#include <mgbase/container/circular_buffer.hpp>

namespace mgcom {
namespace mpi3 {

class mpi3_completer
{
    static const index_t max_completion_count = 16;
    
public:
    void initialize()
    {
    
    }
    
    void finalize()
    {
    
    }
    
    mpi::mpi_completer& get_mpi1_completer() {
        return mpi1_;
    }
    
    MPI_Win get_win() {
        return win_;
    }
    
    bool full() {
        return completions_.full();
    }
    
    void add_completion(const mgbase::operation& on_complete)
    {
        completions_.push_back(on_complete);
    }
    
    void flush()
    {
        
    }
    
    void poll_on_this_thread()
    {
        mpi1_.poll_on_this_thread();
    }

private:
    mpi::mpi_completer mpi1_;
    
    mgbase::circular_buffer<mgbase::operation> completions_;
    MPI_Win win_;
};

} // namespace mpi3
} // namespace mgcom


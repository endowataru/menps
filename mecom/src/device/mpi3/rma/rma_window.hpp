
#pragma once

#include "device/mpi/mpi.hpp"

namespace menps {
namespace mecom {
namespace mpi3 {

class rma_window
{
public:
    rma_window();
    
    ~rma_window();
    
    rma_window(const rma_window&) = delete;
    rma_window& operator = (const rma_window&) = delete;
    
    MPI_Win get() const noexcept { return win_; }

private:
    //delegator&  del_;
    MPI_Win     win_;
};

} // namespace mpi3
} // namespace mecom
} // namespace menps


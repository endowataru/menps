
#pragma once

//#include "common/command/delegator.hpp"
#include "device/mpi/mpi.hpp"

namespace mgcom {
namespace mpi3 {

class rma_window
{
public:
    //rma_window(delegator&);
    rma_window();
    
    ~rma_window();
    
    rma_window(const rma_window&) = delete;
    rma_window& operator = (const rma_window&) = delete;
    
    MPI_Win get() const MGBASE_NOEXCEPT { return win_; }

private:
    //delegator&  del_;
    MPI_Win     win_;
};

} // namespace mpi3
} // namespace mgcom


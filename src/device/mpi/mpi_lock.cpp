
#include "device/mpi/mpi_base.hpp"

namespace mgcom {
namespace mpi_base {

namespace /*unnamed*/ {

mgbase::spinlock lc_;

} // unnamed namespace

void mpi_lock::lock()
{
    lc_.lock();
}

bool mpi_lock::try_lock()
{
    return lc_.try_lock();
}

void mpi_lock::unlock()
{
    lc_.unlock();
}

} // namespace mpi_base
} // namespace mgcom


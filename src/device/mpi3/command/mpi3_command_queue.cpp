
#include "mpi3_command_queue.impl.hpp"

namespace mgcom {

namespace mpi3 {

namespace /*unnamed*/ {

mpi3_command_queue g_mpi3;

} // unnamed namespace

mpi3_command_queue_base& g_queue = g_mpi3;

void initialize_command_queue()
{
    g_mpi3.initialize();
}
void finalize_command_queue()
{
    g_mpi3.finalize();
}

} // namespace mpi3

namespace mpi {

mpi_command_queue_base& g_queue = mpi3::g_mpi3;

} // namespace mpi

} // namespace mgcom


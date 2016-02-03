
#include "mpi1_command_queue.impl.hpp"

namespace mgcom {

namespace mpi1 {

namespace /*unnamed*/ {

mpi1_command_queue g_queue;

} // unnamed namespace

void initialize_command_queue()
{
    g_queue.initialize();
}
void finalize_command_queue()
{
    g_queue.finalize();
}

} // namespace mpi1

namespace mpi {

mpi_command_queue_base& g_queue = mpi1::g_queue;

} // namespace mpi

} // namespace mgcom


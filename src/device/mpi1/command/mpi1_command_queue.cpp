
#include "mpi1_command_queue.impl.hpp"
#include "mpi1_command_queue.hpp"

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

namespace /*unnamed*/ {

mpi1::mpi1_command_queue& g_queue = mpi1::g_queue;

} // unnamed namespace

} // namespace mpi

} // namespace mgcom

#include "device/mpi/command/mpi_interface.impl.hpp"


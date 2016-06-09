
#include "mpi3_command_queue.impl.hpp"
#include "mpi3_command_queue.hpp"

namespace mgcom {

namespace mpi3 {

namespace /*unnamed*/ {

mpi3_command_queue g_queue;

} // unnamed namespace

void initialize_command_queue()
{
    g_queue.initialize();
}
void finalize_command_queue()
{
    g_queue.finalize();
}

} // namespace mpi3

namespace mpi {

mpi3::mpi3_command_queue& g_queue = mpi3::g_queue;

} // namespace mpi

} // namespace mgcom

#include "device/mpi/command/mpi_interface.impl.hpp"

#include "device/mpi3/collective/collective.impl.hpp"
#include "mpi3_interface.impl.hpp"


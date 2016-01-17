
#include <mgcom.hpp>

#include "device/mpi/mpi_base.hpp"
#include "am.hpp"
#include "receiver.hpp"
#include "sender.hpp"
#include "sender_queue.hpp"

namespace mgcom {
namespace am {

void initialize() {
    mpi_error::check(
        MPI_Comm_dup(MPI_COMM_WORLD, &get_comm())
    );
    
    receiver::initialize();
    sender::initialize();
    
}

void finalize() {
    sender::finalize();
    receiver::finalize();
    
    mpi_error::check(
        MPI_Comm_free(&get_comm())
    );
}

void poll() {
    receiver::poll();
    sender_queue::poll();
}

} // namespace am
} // namespace mgcom


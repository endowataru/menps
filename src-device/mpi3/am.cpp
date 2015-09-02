
#include <mgcom.hpp>
#include "mpi_error.hpp"
#include "am.hpp"
#include "am/receiver.hpp"
#include "am/sender.hpp"
#include "am/sender_queue.hpp"

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

}

}


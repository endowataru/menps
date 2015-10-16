
#include <mgcom.hpp>
#include "common/mpi_error.hpp"

namespace mgcom {

void barrier()
{
    MPI_Request request;
    mpi_error::check(
        MPI_Ibarrier(MPI_COMM_WORLD, &request)
    );
    
    while (true) {
        int flag;
        MPI_Status status;
        mpi_error::check(
            MPI_Test(&request, &flag, &status)
        );
        
        if (flag)
            break;
        
        //std::cout << current_process_id() << "barrier" << std::endl;
        
        am::poll();
    }
}

}


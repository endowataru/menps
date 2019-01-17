
#include <menps/medev2/mpi/mpi.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    int provided = 0;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    
    int num_procs = 0;
    int my_rank = 0;
    
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    
    int recv_buf = 0, send_buf = my_rank + 100;
    
    MPI_Request req = MPI_Request();
    
    MPI_Irecv(&recv_buf, 1, MPI_INT,
        (my_rank+1) % num_procs, 0, MPI_COMM_WORLD, &req);
    
    MPI_Send(&send_buf, 1, MPI_INT,
        (my_rank+(num_procs-1)) % num_procs, 0, MPI_COMM_WORLD);
    
    MPI_Wait(&req, MPI_STATUS_IGNORE);
    
    for (int i = 0; i < num_procs; ++i) {
        if (i == my_rank) {
            std::cout << my_rank << " " << recv_buf << std::endl;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    MPI_Finalize();
    
    return 0;
}


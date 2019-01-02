
#include <mpi.h>
#include <iostream>
#include <myth/myth.h>
#include <stdint.h>
#include <menps/mefdn/profiling/time.hpp>
#include <menps/mefdn/external/fmt.hpp>

int g_num_threads;
int g_num_calls;

void* thread_main(void*)
{
    for (int i = 0; i < g_num_calls; ++i) {
        #if 1
        MPI_Send(nullptr, 0, MPI_BYTE, MPI_PROC_NULL, 0, MPI_COMM_WORLD);
        #else
        MPI_Request req = MPI_Request();
        MPI_Isend(nullptr, 0, MPI_BYTE, MPI_PROC_NULL, 0, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, MPI_STATUS_IGNORE);
        #endif
    }
}

int main(int argc, char* argv[])
{
    int provided = 0;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    
    int num_procs = 0;
    int my_rank = 0;
    
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    
    g_num_threads = atoi(argv[1]);
    g_num_calls = atoi(argv[2]);
    
    auto ths = new myth_thread_t[g_num_threads];
    
    const auto start_sec = menps::mefdn::get_current_sec();
    
    for (int i = 0; i < g_num_threads; ++i) {
        ths[i] = myth_create(thread_main, nullptr);
    }
    for (int i = 0; i < g_num_threads; ++i) {
        void* out = nullptr;
        myth_join(ths[i], &out);
    }
    
    const auto finish_sec = menps::mefdn::get_current_sec();
    
    for (int i = 0; i < num_procs; ++i) {
        if (i == my_rank) {
            const auto diff_sec = finish_sec - start_sec;
            fmt::print("{}: {} sec ({} sec/msg/thread)",
                my_rank, diff_sec, (diff_sec / g_num_calls / g_num_threads));
            std::cout << std::endl;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    MPI_Finalize();
    
    delete[] ths;
    
    return 0;
}


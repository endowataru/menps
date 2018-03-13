
#pragma once

#include <menps/medev2/mpi/mpi_params.hpp>
#include <exception>

namespace menps {
namespace medev2 {
namespace mpi {

struct mpi_error : std::exception
{
    static void check(const int err_code) {
        if (err_code != MPI_SUCCESS) {
            throw mpi_error();
        }
    }
};

//template <typename P>
class direct_requester
{
public:
    explicit direct_requester(int* const argc, char*** const argv)
    {
        int level = 0;
        
        mpi_error::check(
            //MPI_Init(argc, argv)
            MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &level)
        );
    }
    
    direct_requester(const direct_requester&) = delete;
    direct_requester& operator = (const direct_requester&) = delete;
    
    ~direct_requester()
    {
        MPI_Finalize();
    }
    
    void send(const send_params& p)
    {
        mpi_error::check(
            MPI_Send(
                p.buf               // buf
            ,   p.num_bytes         // count
            ,   MPI_BYTE            // datatype
            ,   p.dest_rank         // dest
            ,   p.tag               // tag
            ,   p.comm              // comm
            )
        );
    }
    
    void recv(const recv_params& p)
    {
        mpi_error::check(
            MPI_Recv(
                p.buf               // buf
            ,   p.num_bytes         // count
            ,   MPI_BYTE            // datatype
            ,   p.src_rank          // dest
            ,   p.tag               // tag
            ,   p.comm              // comm
            ,   p.status_result     // status
            )
        );
    }
    
    void probe(const probe_params& p)
    {
        mpi_error::check(
            MPI_Probe(
                p.src_rank
            ,   p.tag
            ,   p.comm
            ,   p.status_result
            )
        );
    }
    
    bool iprobe(const probe_params& p)
    {
        int flag = 0;
        mpi_error::check(
            MPI_Iprobe(
                p.src_rank
            ,   p.tag
            ,   p.comm
            ,   &flag
            ,   p.status_result
            )
        );
        return flag != 0;
    }
    
    void barrier(const barrier_params& p)
    {
        mpi_error::check(
            MPI_Barrier(
                p.comm
            )
        );
    }
    void broadcast(const broadcast_params& p)
    {
        mpi_error::check(
            MPI_Bcast(
                p.ptr
            ,   p.num_bytes
            ,   MPI_BYTE
            ,   p.root
            ,   p.comm
            )
        );
    }
    void allgather(const allgather_params& p)
    {
        mpi_error::check(
            MPI_Allgather(
                p.src
            ,   p.num_bytes
            ,   MPI_BYTE
            ,   p.dest
            ,   p.num_bytes
            ,   MPI_BYTE
            ,   p.comm
            )
        );
    }
    
    int get_count(const MPI_Status& status)
    {
        int count = 0;
        
        mpi_error::check(
            MPI_Get_count(&status, MPI_BYTE, &count)
        );
        
        return count;
    }
    
    MPI_Comm comm_dup(const MPI_Comm old_comm)
    {
        MPI_Comm result;
        
        mpi_error::check(
            MPI_Comm_dup(old_comm, &result)
        );
        
        return result;
    }
    
    void comm_free(MPI_Comm* const comm)
    {
        mpi_error::check(
            MPI_Comm_free(comm)
        );
    }
    
    void get(const get_params& p)
    {
        mpi_error::check(
            MPI_Get(
                p.dest_ptr      // origin_addr
            ,   p.num_bytes     // origin_count
            ,   MPI_BYTE        // origin_datatype
            ,   p.src_rank      // target_rank
            ,   p.src_index     // target_disp
            ,   p.num_bytes     // target_count
            ,   MPI_BYTE        // target_datatype
            ,   p.win           // win
            )
        );
    }
    
    void put(const put_params& p)
    {
        mpi_error::check(
            MPI_Put(
                p.src_ptr       // origin_addr
            ,   p.num_bytes     // origin_count
            ,   MPI_BYTE        // origin_datatype
            ,   p.dest_rank     // target_rank
            ,   p.dest_index    // target_disp
            ,   p.num_bytes     // target_count
            ,   MPI_BYTE        // target_datatype
            ,   p.win           // win
            )
        );
    }
    
    void compare_and_swap(const compare_and_swap_params& p)
    {
        mpi_error::check(
            MPI_Compare_and_swap(
                p.desired_ptr   // origin_addr
            ,   p.expected_ptr  // compare_addr
            ,   p.result_ptr    // result_addr
            ,   p.datatype      // datatype
            ,   p.target_rank   // target_rank
            ,   p.target_index  // target_disp
            ,   p.win
            )
        );
    }
    
    void win_flush_all(const win_flush_all_params& p)
    {
        mpi_error::check(
            MPI_Win_flush_all(p.win)
        );
    }
    
    MPI_Win win_create_dynamic(const win_create_dynamic_params& p)
    {
        MPI_Win win{};
        mpi_error::check(
            MPI_Win_create_dynamic(p.info, p.comm, &win)
        );
        return win;
    }
    
    void win_free(MPI_Win* const win) {
        mpi_error::check(
            MPI_Win_free(win)
        );
    }
    
    void win_attach(const win_attach_params& p) {
        mpi_error::check(
            MPI_Win_attach(p.win, p.base, p.size)
        );
    }
    void win_detach(const win_detach_params& p) {
        mpi_error::check(
            MPI_Win_detach(p.win, p.base)
        );
    }
    void win_lock_all(const win_lock_all_params& p) {
        mpi_error::check(
            MPI_Win_lock_all(p.assert, p.win)
        );
    }
    void win_unlock_all(const win_unlock_all_params& p) {
        mpi_error::check(
            MPI_Win_unlock_all(p.win)
        );
    }
    
    int comm_rank(const MPI_Comm comm) {
        int rank = 0;
        mpi_error::check(
            MPI_Comm_rank(comm, &rank)
        );
        return rank;
    }
    int comm_size(const MPI_Comm comm) {
        int rank = 0;
        mpi_error::check(
            MPI_Comm_size(comm, &rank)
        );
        return rank;  
    }
};

} // namespace mpi
} // namespace medev2
} // namespace menps


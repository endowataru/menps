
#pragma once

#include <menps/medev2/mpi/mpi_params.hpp>
#include <menps/mefdn/logger.hpp>
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

#ifdef MEDEV2_SERIALIZE_MPI_CALLS
    #define MPI_CRITICAL    unique_lock_type lk(this->mtx_);
#else
    #define MPI_CRITICAL
#endif

class direct_requester
{
    using ult_itf_type = medev2::default_ult_itf;
    using mutex_type = typename ult_itf_type::mutex;
    using unique_lock_type = typename ult_itf_type::unique_mutex_lock;
    
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
        MPI_CRITICAL
        
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
        MPI_CRITICAL
        
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
    
    void isend(const isend_params& p)
    {
        MEFDN_LOG_VERBOSE(
            "msg:Call MPI_Isend().\t"
            "buf:0x{:x}\t"
            "count:{}\t"
            "dest_rank:{}\t"
            "tag:{}\t"
        ,   reinterpret_cast<mefdn::intptr_t>(p.buf)
        ,   p.count
        ,   p.dest_rank
        ,   p.tag
        );
        
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Isend(
                p.buf       // buf
            ,   p.count     // count
            ,   p.datatype  // datatype
            ,   p.dest_rank // dest
            ,   p.tag       // tag
            ,   p.comm      // comm
            ,   p.request   // request
            )
        );
    }
    
    void irecv(const irecv_params& p)
    {
        MEFDN_LOG_VERBOSE(
            "msg:Call MPI_Irecv().\t"
            "buf:0x{:x}\t"
            "count:{}\t"
            "src_rank:{}\t"
            "tag:{}\t"
        ,   reinterpret_cast<mefdn::intptr_t>(p.buf)
        ,   p.count
        ,   p.src_rank
        ,   p.tag
        );
        
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Irecv(
                p.buf       // buf
            ,   p.count     // count
            ,   p.datatype  // datatype
            ,   p.src_rank  // source
            ,   p.tag       // tag
            ,   p.comm      // comm
            ,   p.request   // request
            )
        );
    }
    
    void test(const test_params& p)
    {
        MEFDN_LOG_VERBOSE(
            "msg:Call MPI_Test().\t"
            "request:0x{:x}"
        ,   reinterpret_cast<mefdn::intptr_t>(p.request)
        );
        
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Test(
                p.request       // request
            ,   p.flag_result   // flag
            ,   p.status_result // status
            )
        );
    }
    
    void probe(const probe_params& p)
    {
        MPI_CRITICAL
        
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
        MPI_CRITICAL
        
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
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Barrier(
                p.comm
            )
        );
    }
    void broadcast(const broadcast_params& p)
    {
        MPI_CRITICAL
        
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
        MPI_CRITICAL
        
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
        MPI_CRITICAL
        
        int count = 0;
        
        mpi_error::check(
            MPI_Get_count(&status, MPI_BYTE, &count)
        );
        
        return count;
    }
    
    MPI_Comm comm_dup(const MPI_Comm old_comm)
    {
        MPI_CRITICAL
        
        MPI_Comm result;
        
        mpi_error::check(
            MPI_Comm_dup(old_comm, &result)
        );
        
        return result;
    }
    
    void comm_free(MPI_Comm* const comm)
    {
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Comm_free(comm)
        );
    }
    
    void get(const get_params& p)
    {
        MPI_CRITICAL
        
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
        MPI_CRITICAL
        
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
        MPI_CRITICAL
        
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
    
    void rput(const rput_params& p)
    {
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Rput(
                p.origin_addr
            ,   p.origin_count
            ,   p.origin_datatype
            ,   p.target_rank
            ,   p.target_disp
            ,   p.target_count
            ,   p.target_datatype
            ,   p.win
            ,   p.request
            )
        );
    }
    
    void rget(const rget_params& p)
    {
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Rget(
                p.origin_addr
            ,   p.origin_count
            ,   p.origin_datatype
            ,   p.target_rank
            ,   p.target_disp
            ,   p.target_count
            ,   p.target_datatype
            ,   p.win
            ,   p.request
            )
        );
    }
    
    void rget_accumulate(const rget_accumulate_params& p)
    {
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Rget_accumulate(
                p.origin_addr
            ,   p.origin_count
            ,   p.origin_datatype
            ,   p.result_addr
            ,   p.result_count
            ,   p.result_datatype
            ,   p.target_rank
            ,   p.target_disp
            ,   p.target_count
            ,   p.target_datatype
            ,   p.op
            ,   p.win
            ,   p.request
            )
        );
    }
    
    void win_flush_all(const win_flush_all_params& p)
    {
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Win_flush_all(p.win)
        );
    }
    void win_flush_local_all(const win_flush_all_params& p)
    {
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Win_flush_local_all(p.win)
        );
    }
    
    void win_flush_local(const win_flush_local_params& p)
    {
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Win_flush_local(p.rank, p.win)
        );
    }
    
    MPI_Win win_create_dynamic(const win_create_dynamic_params& p)
    {
        MPI_CRITICAL
        
        MPI_Win win{};
        mpi_error::check(
            MPI_Win_create_dynamic(p.info, p.comm, &win)
        );
        return win;
    }
    
    void win_free(MPI_Win* const win) {
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Win_free(win)
        );
    }
    
    void win_attach(const win_attach_params& p) {
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Win_attach(p.win, p.base, p.size)
        );
    }
    void win_detach(const win_detach_params& p) {
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Win_detach(p.win, p.base)
        );
    }
    void win_lock_all(const win_lock_all_params& p) {
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Win_lock_all(p.assert, p.win)
        );
    }
    void win_unlock_all(const win_unlock_all_params& p) {
        MPI_CRITICAL
        
        mpi_error::check(
            MPI_Win_unlock_all(p.win)
        );
    }
    
    int comm_rank(const MPI_Comm comm) {
        MPI_CRITICAL
        
        int rank = 0;
        mpi_error::check(
            MPI_Comm_rank(comm, &rank)
        );
        return rank;
    }
    int comm_size(const MPI_Comm comm) {
        MPI_CRITICAL
        
        int rank = 0;
        mpi_error::check(
            MPI_Comm_size(comm, &rank)
        );
        return rank;  
    }
    
    #ifdef MEDEV2_SERIALIZE_MPI_CALLS
private:
    mutex_type mtx_;
    #endif
};

} // namespace mpi
} // namespace medev2
} // namespace menps


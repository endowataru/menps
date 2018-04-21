
#pragma once

#include <menps/mecom2/coll/coll_typed.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_mpi_coll
    : public coll_typed<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
public:
    void barrier()
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto comm = self.get_communicator();
        
        MEFDN_LOG_VERBOSE("msg:Call MPI_barrier().");
        
        mi.barrier({ comm });
    }
    
    void untyped_broadcast(
        const proc_id_type  root_proc
    ,   void* const         ptr
    ,   const size_type     num_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto comm = self.get_communicator();
        
        MEFDN_LOG_VERBOSE(
            "msg:Call MPI_Bcast().\t"
            "root_proc:{}\t"
            "ptr:0x{:x}\t"
            "num_bytes:{}\t"
        ,   root_proc
        ,   reinterpret_cast<mefdn::intptr_t>(ptr)
        ,   num_bytes
        );
        
        mi.broadcast({ ptr, num_bytes, root_proc, comm });
    }
    
    void untyped_allgather(
        const void* const   src_ptr
    ,   void* const         dest_ptr
    ,   const size_type     num_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto comm = self.get_communicator();
        const auto num_bytes_int = static_cast<int>(num_bytes);
        
        MEFDN_LOG_VERBOSE(
            "msg:Call MPI_Allgather().\t"
            "src_ptr:0x{:x}\t"
            "dest_ptr:0x{:x}\t"
            "num_bytes:{}\t"
        ,   reinterpret_cast<mefdn::intptr_t>(src_ptr)
        ,   reinterpret_cast<mefdn::intptr_t>(dest_ptr)
        ,   num_bytes
        );
        
        mi.allgather({ src_ptr, dest_ptr, num_bytes_int, comm });
    }
    
    void untyped_alltoall(
        const void* const   src_ptr
    ,   void* const         dest_ptr
    ,   const size_type     num_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto comm = self.get_communicator();
        
        MEFDN_LOG_VERBOSE(
            "msg:Call MPI_Alltoall().\t"
            "src_ptr:0x{:x}\t"
            "dest_ptr:0x{:x}\t"
            "num_bytes:{}\t"
        ,   reinterpret_cast<mefdn::intptr_t>(src_ptr)
        ,   reinterpret_cast<mefdn::intptr_t>(dest_ptr)
        ,   num_bytes
        );
        
        mi.alltoall({ src_ptr, dest_ptr, num_bytes, comm });
    }
    
};

class mpi_coll;

struct mpi_coll_policy
{
    using derived_type = mpi_coll;
    using size_type = mefdn::size_t;
    using proc_id_type = int;
};

class mpi_coll
    : public basic_mpi_coll<mpi_coll_policy>
{
public:
    template <typename Conf>
    explicit mpi_coll(Conf&& conf)
        : req_(conf.req)
        , comm_(conf.comm)
        , rank_(conf.req.comm_rank(conf.comm))
        , num_ranks_(conf.req.comm_size(conf.comm))
    { }
    
    medev2::mpi::direct_requester& get_mpi_interface() const noexcept {
        return this->req_;
    }
    MPI_Comm get_communicator() const noexcept {
        return this->comm_;
    }
    
    // TODO: remove these
    int current_process_id() const noexcept {
        return this->rank_;
    }
    int number_of_processes() const noexcept {
        return this->num_ranks_;
    }
    
    // newer functions
    int this_proc_id() const noexcept {
        return this->rank_;
    }
    int get_num_procs() const noexcept {
        return this->num_ranks_;
    }
    
private:
    medev2::mpi::direct_requester& req_;
    const MPI_Comm comm_;
    const int rank_;
    const int num_ranks_;
};

inline mpi_coll make_mpi_coll(medev2::mpi::direct_requester& req, MPI_Comm comm)
{
    struct conf {
        medev2::mpi::direct_requester& req;
        MPI_Comm comm;
    };
    return mpi_coll(conf{ req, comm });
}

} // namespace mecom2
} // namespace menps


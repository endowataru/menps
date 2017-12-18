
#pragma once

#include <menps/mecom2/coll/coll_typed.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_mpi_coll
    : public coll_typed<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using size_type = typename P::size_type;
    
public:
    void barrier()
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto comm = self.get_communicator();
        
        mi.barrier({ comm });
    }
    
    void untyped_allgather(
        const void* const   src_ptr
    ,   void* const         dest_ptr
    ,   const size_type     num_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto comm = self.get_communicator();
        
        mi.allgather({ src_ptr, dest_ptr, num_bytes, comm });
    }
    
    void untyped_alltoall(
        const void* const   src_ptr
    ,   void* const         dest_ptr
    ,   const size_type     num_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto comm = self.get_communicator();
        
        mi.alltoall({ src_ptr, dest_ptr, num_bytes, comm });
    }
    
};

class mpi_coll;

struct mpi_coll_policy
{
    using derived_type = mpi_coll;
    using size_type = mefdn::size_t;
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
    
    int current_process_id() const noexcept {
        return this->rank_;
    }
    int number_of_processes() const noexcept {
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


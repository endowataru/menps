
#pragma once

#include <menps/mecom2/coll/mpi/basic_mpi_coll.hpp>

namespace menps {
namespace mecom2 {

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


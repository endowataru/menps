
#pragma once

#include <menps/mecom2/coll/mpi/basic_mpi_coll.hpp>
#include <menps/medev2/mpi/mpi_datatype.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class mpi_coll
    : public basic_mpi_coll<P>
{
    using mpi_itf_type = typename P::mpi_itf_type;
    using mpi_facade_type = typename mpi_itf_type::mpi_facade_type;
    
public:
    template <typename Conf>
    explicit mpi_coll(Conf&& conf)
        : req_(conf.req)
        , comm_(conf.comm)
        , rank_(0)
        , num_ranks_(0)
    {
        conf.req.comm_rank({ conf.comm, &this->rank_ });
        conf.req.comm_size({ conf.comm, &this->num_ranks_ });
    }
    
    mpi_facade_type& get_mpi_interface() const noexcept {
        return this->req_;
    }
    MPI_Comm get_communicator() const noexcept {
        return this->comm_;
    }
    
    int this_proc_id() const noexcept {
        return this->rank_;
    }
    int get_num_procs() const noexcept {
        return this->num_ranks_;
    }
    
private:
    mpi_facade_type& req_;
    const MPI_Comm comm_;
    int rank_;
    int num_ranks_;
};

template <typename P>
using mpi_coll_ptr = mefdn::unique_ptr<mpi_coll<P>>;

template <typename P>
inline mpi_coll_ptr<P>
make_mpi_coll(
    typename P::mpi_itf_type::mpi_facade_type&  req
,   const MPI_Comm                              comm
) {
    struct conf {
        typename P::mpi_itf_type::mpi_facade_type& req;
        MPI_Comm comm;
    };
    return mefdn::make_unique<mpi_coll<P>>(conf{ req, comm });
}

template <typename MpiItf>
struct mpi_coll_policy
{
    using derived_type = mpi_coll<mpi_coll_policy>;
    
    using mpi_itf_type = MpiItf;
    
    using size_type = mefdn::size_t;
    using proc_id_type = int;
    
    template <typename T>
    static MPI_Datatype get_mpi_datatype() {
        return medev2::mpi::get_datatype<T>()();
    }
};

} // namespace mecom2
} // namespace menps


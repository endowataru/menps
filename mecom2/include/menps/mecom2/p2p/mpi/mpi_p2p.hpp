
#pragma once

#include <menps/mecom2/p2p/mpi/basic_mpi_p2p.hpp>
#include <menps/medev2/mpi.hpp>

namespace menps {
namespace mecom2 {

class mpi_p2p;

struct mpi_p2p_policy
{
    using derived_type = mpi_p2p;
    
    using size_type = mefdn::size_t;
    using proc_id_type = int;
    using tag_type = int;
    
    using ult_itf_type = default_ult_itf;
};

class mpi_p2p
    : public basic_mpi_p2p<mpi_p2p_policy>
{
    using mpi_itf_type = medev2::mpi::direct_requester;
    
public:
    template <typename Conf>
    explicit mpi_p2p(Conf&& conf)
        : itf_(conf.req)
        , comm_(conf.comm)
    { }
    
    mpi_itf_type& get_mpi_itf() {
        return itf_;
    }
    MPI_Comm get_comm() const noexcept {
        return this->comm_;
    }
    
private:
    medev2::mpi::direct_requester& itf_;
    const MPI_Comm comm_;
};

inline mpi_p2p make_mpi_p2p(
    medev2::mpi::direct_requester&  req
,   const MPI_Comm                  comm
) {
    struct conf {
        medev2::mpi::direct_requester&  req;
        MPI_Comm                        comm;
    };
    return mpi_p2p(conf{ req, comm });
}

} // namespace mecom2
} // namespace menps


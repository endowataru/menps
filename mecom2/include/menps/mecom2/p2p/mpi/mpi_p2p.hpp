
#pragma once

#include <menps/mecom2/p2p/mpi/basic_mpi_p2p.hpp>
#include <menps/medev2/mpi.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class mpi_p2p;

template <typename P>
class mpi_p2p
    : public basic_mpi_p2p<P>
{
    using mpi_itf_type = typename P::mpi_itf_type;
    using mpi_facade_type = typename mpi_itf_type::mpi_facade_type;
    
public:
    template <typename Conf>
    explicit mpi_p2p(Conf&& conf)
        : mf_(conf.mf)
        , comm_(conf.comm)
    { }
    
    mpi_facade_type& get_mpi_facade() {
        return mf_;
    }
    MPI_Comm get_comm() const noexcept {
        return this->comm_;
    }
    
private:
    mpi_facade_type&    mf_;
    const MPI_Comm      comm_;
};

template <typename P>
using mpi_p2p_ptr = mefdn::unique_ptr<mpi_p2p<P>>;

template <typename P>
inline mpi_p2p_ptr<P> make_mpi_p2p(
    typename P::mpi_itf_type::mpi_facade_type&  mf
,   const MPI_Comm                              comm
) {
    struct conf {
        typename P::mpi_itf_type::mpi_facade_type&  mf;
        MPI_Comm                                    comm;
    };
    return mefdn::make_unique<mpi_p2p<P>>(conf{ mf, comm });
}

template <typename MpiItf>
struct mpi_p2p_policy
{
    using derived_type = mpi_p2p<mpi_p2p_policy>;
    
    using size_type = mefdn::size_t;
    using proc_id_type = int;
    using tag_type = int;
    
    using mpi_itf_type = MpiItf;
    
    using ult_itf_type = typename MpiItf::ult_itf_type;
};

} // namespace mecom2
} // namespace menps



#pragma once

#include <menps/mecom2/rma/rma_itf_id.hpp>

// TODO: Because mpi_based_rma is specializing the interface,
//       we need to include all headers...
#include MECOM2_RMA_ITF_HEADER_SINGLE
#include MECOM2_RMA_ITF_HEADER_MPI
#include MECOM2_RMA_ITF_HEADER_UCT

namespace menps {
namespace mecom2 {

template <mecom2::rma_itf_id_t Id, typename P>
struct mpi_based_rma;

template <typename P>
struct mpi_based_rma<mecom2::rma_itf_id_t::SINGLE, P>
{
    using mpi_itf_type = typename P::mpi_itf_type;
    using mpi_facade_type = typename mpi_itf_type::mpi_facade_type;
    
    mecom2::single_rma_ptr  rma;
    
    template <typename Coll>
    explicit mpi_based_rma(mpi_facade_type& /*mf*/, Coll& /*coll*/)
    {
        rma = mecom2::make_single_rma();
    }
};

template <typename P>
struct mpi_based_rma<mecom2::rma_itf_id_t::MPI, P>
{
    using mpi_itf_type = typename P::mpi_itf_type;
    using mpi_facade_type = typename mpi_itf_type::mpi_facade_type;
    using mpi_rma_policy_type = mecom2::mpi_rma_policy<mpi_itf_type>;
    
    MPI_Comm    comm;
    MPI_Win     win;
    mecom2::mpi_rma_ptr<mpi_rma_policy_type> rma;
    
    template <typename Coll>
    explicit mpi_based_rma(mpi_facade_type& mf, Coll& /*coll*/)
    {
        mf.comm_dup({ MPI_COMM_WORLD, &comm }); // TODO
        mf.win_create_dynamic({ MPI_INFO_NULL, comm, &win });
        
        mf.win_lock_all({ 0, win });
        
        rma = mecom2::make_mpi_rma<mpi_rma_policy_type>(mf, win, MPI_COMM_WORLD);
    }
};

inline int get_procs_per_node() {
    if (const auto str = std::getenv("MECOM2_NUM_PROCS_PER_NODE")) {
        const auto ret = std::atoi(str);
        MEFDN_ASSERT(ret > 0);
        return ret;
    }
    else
        return 1;
}

template <typename P>
struct mpi_based_rma<mecom2::rma_itf_id_t::UCT, P>
{
    using mpi_itf_type = typename P::mpi_itf_type;
    using mpi_facade_type = typename mpi_itf_type::mpi_facade_type;
    using uct_itf_type = typename P::uct_itf_type;
    
    mefdn::unique_ptr<mecom2::uct_rma_resource<uct_itf_type>>   rma_res;
    mecom2::uct_rma<mecom2::uct_rma_policy<uct_itf_type>>*      rma;
    
    template <typename Coll>
    explicit mpi_based_rma(mpi_facade_type& /*mf*/, Coll& coll)
    {
        const auto procs_per_node =
            static_cast<mefdn::size_t>(get_procs_per_node());
        
        // TODO
        const char tl_name[] = "rc_mlx5";
        //const char tl_name[] = "rc";
        #if 1
        const auto hca_num = coll.this_proc_id() % procs_per_node;
        const auto dev_name_str = fmt::format("mlx5_{}:1", hca_num);
        const auto dev_name = dev_name_str.c_str();
        #else
        const char dev_name[] = "mlx5_0:1";
        #endif
        
        MEFDN_LOG_DEBUG(
            "msg:Set up UCT."
            "dev_name:{}"
        ,   dev_name
        );
        
        rma_res = mecom2::make_uct_rma_resource<uct_itf_type>(tl_name, dev_name, coll);
        rma = rma_res->rma.get();
    }
};

template <mecom2::rma_itf_id_t Id, typename P, typename Coll>
inline mefdn::unique_ptr<mpi_based_rma<Id, P>>
make_mpi_based_rma(typename P::mpi_itf_type::mpi_facade_type& mf, Coll& coll)
{
    return mefdn::make_unique<mpi_based_rma<Id, P>>(mf, coll);
}

} // namespace mecom2 
} // namespace menps


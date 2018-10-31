
#pragma once

#include <menps/medsm2/com/dsm_rma.hpp>
#include <menps/medsm2/com/dsm_com_itf.hpp>
#include <menps/mecom2/com/mpi/mpi.hpp>

namespace menps {
namespace medsm2 {

class dsm_com_creator
{
    using mpi_itf_type =
        mecom2::get_mpi_itf_type_t<mecom2::mpi_id_t::MEDSM2_COM_MPI>;
    
    using mpi_facade_type = typename mpi_itf_type::mpi_facade_type;
    
    using mpi_coll_policy_type = mecom2::mpi_coll_policy<mpi_itf_type>;
    using mpi_p2p_policy_type = mecom2::mpi_p2p_policy<mpi_itf_type>;
    
    static constexpr mecom2::rma_id_t used_rma_id = mecom2::rma_id_t::MEDSM2_COM_RMA;
    
    using rma_type = mecom2::get_rma_type_t<used_rma_id, mpi_itf_type>;
    
    using mpi_coll_type = mecom2::mpi_coll<mpi_coll_policy_type>;
    using mpi_p2p_type = mecom2::mpi_p2p<mpi_p2p_policy_type>;
    
public:
    using dsm_com_itf_type =
        dsm_com_itf<dsm_com_policy<rma_type, mpi_coll_type, mpi_p2p_type>>;
    
    explicit dsm_com_creator(
        int* const      argc
    ,   char*** const   argv
    ) {
        this->mf_ = mefdn::make_unique<mpi_facade_type>(argc, argv);
        
        // Duplicate communicators.
        this->mf_->comm_dup({ MPI_COMM_WORLD, &this->coll_comm_ });
        this->mf_->comm_dup({ MPI_COMM_WORLD, &this->p2p_comm_ });
        
        this->coll_ = mecom2::make_mpi_coll<mpi_coll_policy_type>(*this->mf_, this->coll_comm_);
        this->p2p_ = mecom2::make_mpi_p2p<mpi_p2p_policy_type>(*this->mf_, this->p2p_comm_);
        
        this->rma_info_ = make_dsm_rma_info<used_rma_id, mpi_itf_type>(*this->mf_, *this->coll_);
        
        this->com_itf_ = mefdn::make_unique<dsm_com_itf_type>(
            dsm_com_itf_type::conf_t{
                *this->rma_info_->rma
            ,   *this->coll_
            ,   *this->p2p_
            });
    }
    
    dsm_com_itf_type& get_dsm_com_itf() {
        return *this->com_itf_;
    }
    
private:
    mefdn::unique_ptr<mpi_facade_type> mf_;
    MPI_Comm coll_comm_ = MPI_COMM_NULL;
    MPI_Comm p2p_comm_ = MPI_COMM_NULL;
    mecom2::mpi_coll_ptr<mpi_coll_policy_type>  coll_;
    mecom2::mpi_p2p_ptr<mpi_p2p_policy_type>    p2p_;
    mefdn::unique_ptr<dsm_rma_info<used_rma_id, mpi_itf_type>>     rma_info_;
    mefdn::unique_ptr<dsm_com_itf_type>         com_itf_;
};

} // namespace medsm2
} // namespace menps

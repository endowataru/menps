
#pragma once

#include <menps/medsm2/com/dsm_rma.hpp>
#include <menps/medsm2/com/dsm_com_itf.hpp>
#include <menps/mecom2/coll/mpi/mpi_coll.hpp>
#include <menps/mecom2/p2p/mpi/mpi_p2p.hpp>
#include <cmpth/ult_tag.hpp>
#include <cmpth/ult_ext_itf.hpp>
#ifdef MEFDN_ENABLE_MEULT
    #include <menps/meult/ult_itf_id.hpp>
#endif
#include <menps/mefdn/profiling/clock.hpp> // get_cpu_clock

#ifdef MEFDN_ENABLE_MEULT
    #include MEFDN_PP_CAT(MEULT_ULT_ITF_HEADER_, MEDSM2_ULT_ITF)
#else
    #include MEFDN_PP_CAT(CMPTH_ULT_HEADER_, MEDSM2_ULT_ITF)
#endif
#include MEFDN_PP_CAT(MEDEV2_MPI_ITF_HEADER_, MEDSM2_MPI_ITF)
#include MEFDN_PP_CAT(MEDEV2_UCT_ITF_HEADER_, MEDSM2_UCT_ITF)

namespace menps {
namespace medsm2 {

struct dsm_com_policy_base
{
    using ult_itf_type =
        #ifdef MEFDN_ENABLE_MEULT
        meult::get_ult_itf_type_t<meult::ult_itf_id_t::MEDSM2_ULT_ITF>;
        #else
        cmpth::get_ult_ext_itf_t<cmpth::ult_tag_t::MEDSM2_ULT_ITF>;
        #endif
    
    using mpi_itf_type =
        medev2::get_mpi_itf_type_t<
            medev2::mpi_itf_id_t::MEDSM2_MPI_ITF
        ,   ult_itf_type
        >;
    
    using uct_itf_type =
        medev2::ucx::uct::get_uct_itf_type_t<
            medev2::ucx::uct::uct_itf_id_t::MEDSM2_UCT_ITF
        ,   dsm_com_policy_base
        >;
};

class dsm_com_creator
{
public:
    using ult_itf_type = typename dsm_com_policy_base::ult_itf_type;
    
private:
    using mpi_itf_type = typename dsm_com_policy_base::mpi_itf_type;
    using mpi_facade_type = typename mpi_itf_type::mpi_facade_type;
    
    using mpi_coll_policy_type = mecom2::mpi_coll_policy<mpi_itf_type>;
    using mpi_p2p_policy_type = mecom2::mpi_p2p_policy<mpi_itf_type>;
    
    static constexpr mecom2::rma_itf_id_t used_rma_id = mecom2::rma_itf_id_t::MEDSM2_RMA_ITF;
    
    using rma_type = mecom2::get_rma_itf_type_t<used_rma_id, dsm_com_policy_base>;
    
    using mpi_coll_type = mecom2::mpi_coll<mpi_coll_policy_type>;
    using mpi_p2p_type = mecom2::mpi_p2p<mpi_p2p_policy_type>;
    
public:
    using dsm_com_itf_type =
        dsm_com_itf<dsm_com_policy<rma_type, mpi_coll_type, mpi_p2p_type>>;
    
    explicit dsm_com_creator(
        int* const      argc
    ,   char*** const   argv
    ) {
        this->ult_init_ = mefdn::make_unique<typename ult_itf_type::initializer>();
        
        int provided = 0;
        
        this->mf_ =
            mefdn::make_unique<mpi_facade_type>(
                argc, argv, MPI_THREAD_MULTIPLE, &provided);
        
        MEFDN_ASSERT(provided == MPI_THREAD_MULTIPLE);
        
        // Duplicate communicators.
        this->mf_->comm_dup({ MPI_COMM_WORLD, &this->coll_comm_ });
        this->mf_->comm_dup({ MPI_COMM_WORLD, &this->p2p_comm_ });
        
        this->coll_ = mecom2::make_mpi_coll<mpi_coll_policy_type>(*this->mf_, this->coll_comm_);
        this->p2p_ = mecom2::make_mpi_p2p<mpi_p2p_policy_type>(*this->mf_, this->p2p_comm_);
        this->p2p_lock_ = mecom2::make_mpi_p2p<mpi_p2p_policy_type>(*this->mf_, this->p2p_comm_);
        
        #ifndef MEFDN_ENABLE_MEULT
        ult_itf_type::log_policy::set_state_callback(get_state{ *this });
        #endif
        mefdn::logger::set_state_callback(get_state{ *this });
        
        this->rma_info_ = make_dsm_rma_info<used_rma_id, dsm_com_policy_base>(*this->mf_, *this->coll_);
        
        this->com_itf_ = mefdn::make_unique<dsm_com_itf_type>(
            dsm_com_itf_type::conf_t{
                *this->rma_info_->rma
            ,   *this->coll_
            ,   *this->p2p_
            ,   *this->p2p_lock_
            });
    }
    
    ~dsm_com_creator() {
        mefdn::logger::set_state_callback(mefdn::logger::state_callback_type{});
    }
    
    dsm_com_itf_type& get_dsm_com_itf() {
        return *this->com_itf_;
    }
    
private:
    struct get_state
    {
        explicit get_state(dsm_com_creator& self)
            : self_(self)
        { }
        
        std::string operator() ()
        {
            fmt::MemoryWriter w;
            w.write(
                "proc:{}\tthread:{:x}\tult:{:x}\tlog_id:{}\tclock:{}\t"
            ,   self_.coll_->this_proc_id()
            ,   reinterpret_cast<mefdn::uintptr_t>(pthread_self())
                // TODO: use mefdn::this_thread::get_id()
            ,   reinterpret_cast<mefdn::uintptr_t>(
                    ult_itf_type::this_thread::native_handle()
                )
            ,   this->number_++
            ,   mefdn::get_cpu_clock() // TODO
            );
            return w.str();
        }
        
    private:
        dsm_com_creator& self_;
        mefdn::size_t number_ = 0;
    };
    
    mefdn::unique_ptr<typename ult_itf_type::initializer> ult_init_;
    mefdn::unique_ptr<mpi_facade_type> mf_;
    MPI_Comm coll_comm_ = MPI_COMM_NULL;
    MPI_Comm p2p_comm_ = MPI_COMM_NULL;
    mecom2::mpi_coll_ptr<mpi_coll_policy_type>  coll_;
    mecom2::mpi_p2p_ptr<mpi_p2p_policy_type>    p2p_;
    mecom2::mpi_p2p_ptr<mpi_p2p_policy_type>    p2p_lock_;
    mefdn::unique_ptr<dsm_rma_info<used_rma_id, dsm_com_policy_base>>     rma_info_;
    mefdn::unique_ptr<dsm_com_itf_type>         com_itf_;
};

} // namespace medsm2
} // namespace menps


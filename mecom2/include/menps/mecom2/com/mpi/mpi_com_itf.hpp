
#pragma once

#include <menps/mecom2/com/mpi/mpi_based_rma.hpp>
#include <menps/mecom2/coll/mpi/mpi_coll.hpp>
#include <menps/mecom2/p2p/mpi/mpi_p2p.hpp>
#include <menps/mecom2/rpc/mpi/mpi_rpc.hpp>
#include <menps/mecom2/rma/alltoall_buffer.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_mpi_com_itf
{
    using mpi_itf_type = typename P::mpi_itf_type;
    using mpi_facade_type = typename mpi_itf_type::mpi_facade_type;
    
    using rma_info_type = typename P::rma_info_type;
    
    using size_type = mefdn::size_t;
    
public:
    using ult_itf_type = typename P::ult_itf_type;
    
    using coll_itf_type = typename P::coll_itf_type;
    using p2p_itf_type = typename P::p2p_itf_type;
    using rpc_itf_type = typename P::rpc_itf_type;
    using rma_itf_type = typename P::rma_itf_type;
    
    using proc_id_type = typename coll_itf_type::proc_id_type;
    
    explicit basic_mpi_com_itf(
        int* const      argc
    ,   char*** const   argv
    ) {
        this->ult_init_ = mefdn::make_unique<typename ult_itf_type::initializer>();
        
        const int required = MPI_THREAD_MULTIPLE;
        int provided = 0;
        this->mf_ =
            mefdn::make_unique<mpi_facade_type>(
                argc, argv, required, &provided
            );
        
        MEFDN_ASSERT(provided == required); // TODO
        
        this->mf_->comm_dup({ MPI_COMM_WORLD, &this->coll_comm_ });
        this->mf_->comm_dup({ MPI_COMM_WORLD, &this->p2p_comm_ });
        // TODO: Specific to medsm2
        this->mf_->comm_dup({ MPI_COMM_WORLD, &this->p2p_lock_comm_ });
        this->mf_->comm_dup({ MPI_COMM_WORLD, &this->rpc_comm_ });
        
        this->coll_ = P::make_coll(*this->mf_, this->coll_comm_);
        this->p2p_ = P::make_p2p(*this->mf_, this->p2p_comm_);
        // TODO: Specific to medsm2
        this->p2p_lock_ = P::make_p2p(*this->mf_, this->p2p_lock_comm_);
        
        this->rpc_ = P::make_rpc(*this->mf_, this->rpc_comm_);
        
        ult_itf_type::log_policy::set_state_callback(get_state{ *this });
        mefdn::logger::set_state_callback(get_state{ *this });
        
        this->rma_info_ = P::make_rma_info(*this->mf_, *this->coll_);
        
        this->proc_id_ = this->coll_->this_proc_id();
        this->num_procs_ = this->coll_->get_num_procs();
    }
    
    ~basic_mpi_com_itf() {
        mefdn::logger::set_state_callback(mefdn::logger::state_callback_type{});
    }
    
    coll_itf_type& get_coll() const noexcept { return *this->coll_; }
    p2p_itf_type& get_p2p() const noexcept { return *this->p2p_; }
    p2p_itf_type& get_p2p_lock() const noexcept { return *this->p2p_lock_; }
    rpc_itf_type& get_rpc() const noexcept { *this->rpc_; }
    rma_itf_type& get_rma() const noexcept { return *this->rma_info_->rma; }
    
    proc_id_type this_proc_id() { return this->proc_id_; }
    proc_id_type get_num_procs() { return this->num_procs_; }
    
    template <typename Elem>
    alltoall_buffer<rma_itf_type, Elem>
    make_alltoall_buffer(const size_type num_elems_per_proc)
    {
        alltoall_buffer<rma_itf_type, Elem> buf;
        buf.coll_make(this->get_rma(), this->get_coll(), num_elems_per_proc);
        return buf;
    }
    
private:
    struct get_state
    {
        explicit get_state(basic_mpi_com_itf& self)
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
        basic_mpi_com_itf& self_;
        mefdn::size_t number_ = 0;
    };
    
    mefdn::unique_ptr<typename ult_itf_type::initializer> ult_init_;
    
    mefdn::unique_ptr<mpi_facade_type> mf_;
    MPI_Comm coll_comm_ = MPI_COMM_NULL;
    MPI_Comm p2p_comm_ = MPI_COMM_NULL;
    MPI_Comm p2p_lock_comm_ = MPI_COMM_NULL;
    MPI_Comm rpc_comm_ = MPI_COMM_NULL;
    
    mefdn::unique_ptr<coll_itf_type>    coll_;
    mefdn::unique_ptr<p2p_itf_type>     p2p_;
    mefdn::unique_ptr<p2p_itf_type>     p2p_lock_;
    mefdn::unique_ptr<rpc_itf_type>     rpc_;
    mefdn::unique_ptr<rma_info_type>    rma_info_;
    
    proc_id_type proc_id_ = 0;
    proc_id_type num_procs_ = 0;
};


template <typename P>
struct mpi_com_itf_policy
{
    using mpi_itf_type = typename P::mpi_itf_type;
    using mpi_facade_type = typename mpi_itf_type::mpi_facade_type;
    
    using coll_policy_type = mpi_coll_policy<mpi_itf_type>;
    using coll_itf_type = mpi_coll<coll_policy_type>;
    
    using p2p_policy_type = mpi_p2p_policy<mpi_itf_type>;
    using p2p_itf_type = mpi_p2p<p2p_policy_type>;
    
    using rma_info_type = mpi_based_rma<P::rma_id, P>;
    using rma_itf_type = get_rma_itf_type_t<P::rma_id, P>;
    
    using rpc_itf_type = mpi_rpc<mpi_itf_type>;
    
    using ult_itf_type = typename P::ult_itf_type;
    
    static mefdn::unique_ptr<coll_itf_type> make_coll(mpi_facade_type& mf, const MPI_Comm comm) {
        return make_mpi_coll<coll_policy_type>(mf, comm);
    }
    static mefdn::unique_ptr<p2p_itf_type> make_p2p(mpi_facade_type& mf, const MPI_Comm comm) {
        return make_mpi_p2p<p2p_policy_type>(mf, comm);
    }
    static mefdn::unique_ptr<rma_info_type> make_rma_info(mpi_facade_type& mf, coll_itf_type& coll) {
        return make_mpi_based_rma<P::rma_id, P>(mf, coll);
    }
    static mefdn::unique_ptr<rpc_itf_type> make_rpc(mpi_facade_type& mf, const MPI_Comm comm) {
        return make_mpi_rpc<mpi_itf_type>(mf, comm);
    }
};

template <typename P>
using mpi_com_itf = basic_mpi_com_itf<mpi_com_itf_policy<P>>;

} // namespace mecom2
} // namespace menps


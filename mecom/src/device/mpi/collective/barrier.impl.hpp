
#pragma once

#include <menps/mecom/collective.hpp>
#include <menps/mecom/rma.hpp>
#include "collective.hpp"
#include <menps/mefdn/arithmetic.hpp>
#include <menps/mefdn/logger.hpp>

#include "device/mpi/mpi_base.hpp"

namespace menps {
namespace mecom {
namespace mpi {
namespace collective {

namespace /*unnamed*/ {

#if 0
struct barrier_cb {
    MEFDN_CONTINUATION(void)   cont;
    void*                       request;
};
#endif

class barrier_impl
{
public:
    /*
     * Implements "Dissemination Barrier".
     *
     * Reference: https://6xq.net/barrier-intro/
     */
    
    barrier_impl() : initialized_(false) { }
    
    void initialize(mpi_interface& mi)
    {
        mi_ = &mi;
        
        comm_ = mi.comm_dup(MPI_COMM_WORLD, "MECOM_COMM_COLLECTIVE");
        
        required_rounds_ = static_cast<index_t>(
            mefdn::ceil_log2(mi_->get_num_ranks(comm_))
        );
        
        initialized_ = true;
        
        MEFDN_LOG_DEBUG("msg:Initialied a barrier.\trounds:{}", required_rounds_);
    }
    
    void barrier()
    {
        auto& impl = *this;
        MEFDN_ASSERT(impl.initialized_);
        
        impl.round_ = 0;
        
        for ( ; impl.round_ < impl.required_rounds_ ; ++impl.round_)
        {
            //impl.finished_.store(false, mefdn::memory_order_relaxed);
            
            const auto current_proc = impl.mi_->get_current_rank(impl.comm_);
            const auto num_procs = impl.mi_->get_num_ranks(impl.comm_);
            
            const mecom::process_id_t diff = 1 << impl.round_;
            
            // Calculate the destination process.
            const mecom::process_id_t dest_proc
                = static_cast<process_id_t>((current_proc + diff) % num_procs);
            
            const mecom::process_id_t src_proc
                = static_cast<process_id_t>((current_proc + num_procs - diff) % num_procs);
            
            impl.mi_->send_async(
                nullptr
            ,   0
            ,   static_cast<int>(dest_proc)
            ,   0
            ,   impl.comm_
            ,   mefdn::make_callback_empty()
            );
            
            impl.mi_->recv(
                nullptr
            ,   0
            ,   static_cast<int>(src_proc)
            ,   0
            ,   impl.comm_
            ,   MPI_STATUS_IGNORE
            );
        }
    };
    
    
    #if 0
    template <barrier_impl& impl>
    class handlers
    {
        typedef mefdn::deferred<void>  result_type;
        typedef barrier_cb              cb_type;
        
    public:
        static result_type start(cb_type& cb)
        {
            MEFDN_ASSERT(impl.initialized_);
            
            impl.round_ = 0;
            
            return loop(cb);
        }
        
    private:
        static result_type loop(cb_type& cb)
        {
            if (impl.round_ >= impl.required_rounds_)
                return mefdn::make_ready_deferred();
            
            impl.finished_.store(false, mefdn::memory_order_relaxed);
            
            const auto current_proc = impl.mi_->get_current_rank(impl.comm_);
            const auto num_procs = impl.mi_->get_num_ranks(impl.comm_);
            
            const mecom::process_id_t diff = 1 << impl.round_;
            
            // Calculate the destination process.
            const mecom::process_id_t dest_proc
                = static_cast<process_id_t>((current_proc + diff) % num_procs);
            
            const mecom::process_id_t src_proc
                = static_cast<process_id_t>((current_proc + num_procs - diff) % num_procs);
            
            impl.mi_->send_async(
                nullptr
            ,   0
            ,   static_cast<int>(dest_proc)
            ,   0
            ,   impl.comm_
            ,   mefdn::make_callback_empty()
            );
            
            impl.mi_->recv_async(
                nullptr
            ,   0
            ,   static_cast<int>(src_proc)
            ,   0
            ,   impl.comm_
            ,   MPI_STATUS_IGNORE
            ,   mefdn::make_callback_store_release(&impl.finished_, MEFDN_NONTYPE(true))
            );
            
            return test(cb);
        }
        
        static result_type test(cb_type& cb)
        {
            if (impl.finished_.load(mefdn::memory_order_acquire)) {
                ++impl.round_;
                return loop(cb);
            }
            else
                return mefdn::make_deferred(MEFDN_MAKE_INLINED_FUNCTION_TEMPLATE(&handlers::test), cb);
        }
    };
    #endif
    
    MPI_Comm get_communicator() const noexcept
    {
        return comm_;
    }

private:
    bool initialized_;
    mecom::process_id_t round_;
    index_t required_rounds_;
    MPI_Comm comm_;
    mefdn::atomic<bool> finished_;
    
    mpi_interface* mi_;
};

} // unnamed namespace

} // namespace collective
} // namespace mpi
} // namespace mecom
} // namespace menps


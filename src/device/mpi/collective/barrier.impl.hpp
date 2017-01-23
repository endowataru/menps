
#pragma once

#include <mgcom/collective.hpp>
#include <mgcom/rma.hpp>
#include "collective.hpp"
#include <mgbase/arithmetic.hpp>
#include <mgbase/logging/logger.hpp>

#include "device/mpi/mpi_base.hpp"

namespace mgcom {
namespace mpi {
namespace collective {

namespace /*unnamed*/ {

struct barrier_cb {
    MGBASE_CONTINUATION(void)   cont;
    void*                       request;
};

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
        
        comm_ = mi.comm_dup(MPI_COMM_WORLD, "MGCOM_COMM_COLLECTIVE");
        
        required_rounds_ = static_cast<index_t>(
            mgbase::ceil_log2(mi_->get_num_ranks(comm_))
        );
        
        initialized_ = true;
        
        MGBASE_LOG_DEBUG("msg:Initialied a barrier.\trounds:{}", required_rounds_);
    }
    
    template <barrier_impl& impl>
    class handlers
    {
        typedef mgbase::deferred<void>  result_type;
        typedef barrier_cb              cb_type;
        
    public:
        static result_type start(cb_type& cb)
        {
            MGBASE_ASSERT(impl.initialized_);
            
            impl.round_ = 0;
            
            return loop(cb);
        }
        
    private:
        static result_type loop(cb_type& cb)
        {
            if (impl.round_ >= impl.required_rounds_)
                return mgbase::make_ready_deferred();
            
            impl.finished_.store(false, mgbase::memory_order_relaxed);
            
            const auto current_proc = impl.mi_->get_current_rank(impl.comm_);
            const auto num_procs = impl.mi_->get_num_ranks(impl.comm_);
            
            const mgcom::process_id_t diff = 1 << impl.round_;
            
            // Calculate the destination process.
            const mgcom::process_id_t dest_proc
                = static_cast<process_id_t>((current_proc + diff) % num_procs);
            
            const mgcom::process_id_t src_proc
                = static_cast<process_id_t>((current_proc + num_procs - diff) % num_procs);
            
            impl.mi_->send_async(
                MGBASE_NULLPTR
            ,   0
            ,   static_cast<int>(dest_proc)
            ,   0
            ,   impl.comm_
            ,   mgbase::make_callback_empty()
            );
            
            impl.mi_->recv_async(
                MGBASE_NULLPTR
            ,   0
            ,   static_cast<int>(src_proc)
            ,   0
            ,   impl.comm_
            ,   MPI_STATUS_IGNORE
            ,   mgbase::make_callback_store_release(&impl.finished_, MGBASE_NONTYPE(true))
            );
            
            return test(cb);
        }
        
        static result_type test(cb_type& cb)
        {
            if (impl.finished_.load(mgbase::memory_order_acquire)) {
                ++impl.round_;
                return loop(cb);
            }
            else
                return mgbase::make_deferred(MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(&handlers::test), cb);
        }
    };
    
    MPI_Comm get_communicator() const MGBASE_NOEXCEPT
    {
        return comm_;
    }

private:
    bool initialized_;
    mgcom::process_id_t round_;
    index_t required_rounds_;
    MPI_Comm comm_;
    mgbase::atomic<bool> finished_;
    
    mpi_interface* mi_;
};

} // unnamed namespace

} // namespace collective
} // namespace mpi
} // namespace mgcom


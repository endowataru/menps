
#pragma once

#include <mgcom/collective.hpp>
#include <mgcom/rma.hpp>
#include "collective.hpp"
#include <mgbase/arithmetic.hpp>
#include <mgbase/logging/logger.hpp>

#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_call.hpp"

namespace mgcom {
namespace collective {

namespace detail {

namespace /*unnamed*/ {

class barrier_impl
{
public:
    /*
     * Implements "Dissemination Barrier".
     *
     * Reference: https://6xq.net/barrier-intro/
     */
    
    barrier_impl() : initialized_(false) { }
    
    void initialize()
    {
        required_rounds_ = static_cast<index_t>(
            mgbase::ceil_log2(mgcom::number_of_processes())
        );
        
        comm_ = mpi::comm_dup(MPI_COMM_WORLD);
        
        initialized_ = true;
        
        MGBASE_LOG_DEBUG("msg:Initialied a barrier.\trounds:{}", required_rounds_);
    }
    
    template <barrier_impl& impl>
    class handlers
    {
        typedef mgbase::deferred<void>  result_type;
        typedef barrier_cb              cb_type;
        typedef handlers                handlers_type;
        typedef result_type (func_type)(cb_type&);
        
    public:
        static result_type start(cb_type& cb)
        {
            impl.round_ = 0;
            
            return loop(cb);
        }
        
    private:
        static result_type loop(cb_type& cb)
        {
            if (impl.round_ >= impl.required_rounds_)
                return mgbase::make_ready_deferred();
            
            impl.finished_.store(false, mgbase::memory_order_relaxed);
            
            const mgcom::process_id_t current_proc = mgcom::current_process_id();
            const index_t num_procs = mgcom::number_of_processes();
            
            const mgcom::process_id_t diff = 1 << impl.round_;
            
            // Calculate the destination process.
            const mgcom::process_id_t dest_proc
                = static_cast<process_id_t>((current_proc + diff) % num_procs);
            
            const mgcom::process_id_t src_proc
                = static_cast<process_id_t>((current_proc + num_procs - diff) % num_procs);
            
            mpi::isend(
                MGBASE_NULLPTR
            ,   0
            ,   static_cast<int>(dest_proc)
            ,   0
            ,   impl.comm_
            ,   mgbase::make_no_operation()
            );
            
            mpi::irecv(
                MGBASE_NULLPTR
            ,   0
            ,   static_cast<int>(src_proc)
            ,   0
            ,   impl.comm_
            ,   MPI_STATUS_IGNORE
            ,   mgbase::make_operation_store_release(&impl.finished_, true)
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
                return mgbase::make_deferred<func_type, &handlers_type::test>(cb);
        }
    };

private:
    bool initialized_;
    mgcom::process_id_t round_;
    index_t required_rounds_;
    MPI_Comm comm_;
    mgbase::atomic<bool> finished_;
};

} // unnamed namespace

} // namespace detail

} // namespace collective
} // namespace mgcom


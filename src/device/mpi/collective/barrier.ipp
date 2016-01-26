
#pragma once

#include <mgcom/collective.hpp>
#include "collective.hpp"
#include <mgbase/arithmetic.hpp>
#include <mgbase/logging/logger.hpp>

#include "device/mpi/mpi_base.hpp"

namespace mgcom {
namespace collective {

namespace detail {

namespace /*unnamed*/ {

class barrier_impl
{
public:
    /*
     * Implements "Butterfly Buffer".
     * The algorithm itself is not depending on MPI,
     * but MPI is still required to exchange the addresses with MPI_Allgather.
     *
     * Reference: https://6xq.net/barrier-intro/
     */
    
    barrier_impl() : initialized_(false) { }
    
    void initialize()
    {
        required_rounds_ = mgbase::ceil_log2(mgcom::number_of_processes());
        parity_ = 0;
        sense_ = true;
        
        local_ = mgcom::rma::allocate<mgcom::rma::atomic_default_t>(2 * mgcom::number_of_processes());
        buf_ = mgcom::rma::allocate<mgcom::rma::atomic_default_t>(1);
        
        mgcom::rma::local_pointer<mgcom::rma::atomic_default_t>* local_ptrs
            = new mgcom::rma::local_pointer<mgcom::rma::atomic_default_t>[mgcom::number_of_processes()];
        
        {
            mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
            
            // Directly call.
            MPI_Allgather(
                &local_
            ,   sizeof(local_)
            ,   MPI_BYTE
            ,   local_ptrs
            ,   sizeof(local_)
            ,   MPI_BYTE
            ,   MPI_COMM_WORLD
            );
        }
        
        remotes_ = new mgcom::rma::remote_pointer<mgcom::rma::atomic_default_t>[mgcom::number_of_processes()];
        for (process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
            remotes_[proc] = mgcom::rma::use_remote_pointer(proc, local_ptrs[proc]);
        
        initialized_ = true;
        
        MGBASE_LOG_DEBUG("msg:Initialied a barrier.\trounds:{}", required_rounds_);
    }
    
    template <barrier_impl& impl>
    class handlers
    {
        typedef handlers                handlers_type;
        typedef barrier_cb              cb_type;
        typedef mgbase::deferred<void>  result_type;
    
    public:
        static result_type start(cb_type& cb)
        {
            if (number_of_processes() == 1)
                return mgbase::make_ready_deferred();
            
            MGBASE_ASSERT(impl.initialized_);
            impl.round_ = 0;
            
            MGBASE_LOG_DEBUG("msg:Started barrier.");
            
            return loop(cb);
        }
    
    private:
        static result_type loop(cb_type& cb)
        {
            const mgcom::process_id_t current_proc = mgcom::current_process_id();
            const mgcom::process_id_t num_procs = mgcom::number_of_processes();
            
            const mgcom::process_id_t diff = 1 << impl.round_;
            
            // Calculate the destination process.
            const mgcom::process_id_t dest_proc =
                (current_proc + diff) % num_procs;
            
            const mgcom::rma::remote_pointer<mgcom::rma::atomic_default_t>
                remote = impl.remotes_[dest_proc] + (impl.parity_ * number_of_processes()) + impl.round_;
            
            *impl.buf_ = impl.sense_;
            
            return mgbase::add_continuation<result_type (cb_type&), &handlers_type::check>(
                cb
            ,   mgcom::rma::remote_write_nb(
                    impl.cb_write_
                ,   dest_proc
                ,   remote
                ,   impl.buf_
                )
            );
        }
        
        static result_type check(cb_type& cb)
        {
            mgcom::rma::atomic_default_t my_val = *(impl.local_ + (impl.parity_ * number_of_processes()) + impl.round_);
            
            if (my_val == impl.sense_) {
                // Next round.
                ++impl.round_;
                
                if (impl.round_ >= impl.required_rounds_) {
                    MGBASE_LOG_DEBUG("msg:Finished barrier.");
                    
                    if (impl.parity_ == 1) {
                        impl.sense_ = !impl.sense_; 
                    }
                    impl.parity_ = 1 - impl.parity_;
                    
                    return mgbase::make_ready_deferred();
                }
                else {
                    MGBASE_LOG_DEBUG("msg:Next round.");
                    return loop(cb);
                }
            }
            
            mgcom::am::poll();
            
            return mgbase::make_deferred<result_type (cb_type&), &handlers_type::check>(cb);
        }
    };

private:
    bool initialized_;
    mgcom::rma::local_pointer<mgcom::rma::atomic_default_t>      local_;
    mgcom::rma::remote_pointer<mgcom::rma::atomic_default_t>*    remotes_;
    mgcom::rma::local_pointer<mgcom::rma::atomic_default_t>      buf_;
    mgbase::uint32_t parity_;
    bool sense_;
    mgcom::process_id_t round_;
    mgcom::process_id_t required_rounds_;
    mgcom::rma::remote_write_cb cb_write_;
};

} // unnamed namespace

} // namespace detail

} // namespace collective
} // namespace mgcom


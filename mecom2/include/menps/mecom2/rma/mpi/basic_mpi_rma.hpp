
#pragma once

#include <menps/mecom2/rma/rma_typed_itf.hpp>
#include <menps/mecom2/rma/rma_pass_buf_copier.hpp>
#include <menps/mecom2/rma/rma_req_block_itf.hpp>
#include <menps/mecom2/rma/rma_flush_block_itf.hpp>
#include <menps/mecom2/rma/rma_typed_allocator.hpp>
#include <menps/medev2/mpi/mpi_datatype.hpp>
#include <menps/mefdn/logger.hpp>
#ifdef MEDEV2_AVOID_SWITCH_IN_SIGNAL
    #include <menps/mecom2/com/com_signal_state.hpp>
#endif

namespace menps {
namespace mecom2 {

template <typename P>
class basic_mpi_rma
    : public rma_typed_itf<P>
    , public/*protected*/ rma_req_block_itf<P>
    , public/*protected*/ rma_flush_block_itf<P>
    // XXX: GCC 4.8 doesn't accept casting from a protected base class ?
    , public rma_typed_allocator<P>
    , public rma_pass_buf_copier<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using ult_itf_type = typename P::ult_itf_type;
    
    using request_type = typename P::request_type;
    
public:
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
    template <typename T>
    using remote_ptr = typename P::template remote_ptr<T>;
    template <typename T>
    using local_ptr = typename P::template local_ptr<T>;
    template <typename T>
    using public_ptr = typename P::template public_ptr<T>;
    template <typename T>
    using unique_public_ptr = typename P::template unique_public_ptr<T>;
    
    // MPI_Rput()
    using rma_req_block_itf<P>::untyped_write;
    // MPI_Rget()
    using rma_req_block_itf<P>::untyped_read;
    // MPI_Rget_accumulate()
    using rma_req_block_itf<P>::exchange_b;
    // MPI_Compare_and_swap()
    using rma_flush_block_itf<P>::compare_and_swap_b;
    
    void untyped_write_nb(
        const proc_id_type              dest_proc
    ,   const remote_ptr<void>&         dest_rptr
    ,   const local_ptr<const void>&    src_lptr
    ,   const size_type                 num_bytes
    ,   request_type* const             request_result
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        const auto win = self.get_win();
        
        mi.rput({
            src_lptr                    // origin_addr
        ,   static_cast<int>(num_bytes) // origin_count
        ,   MPI_BYTE                    // origin_datatype
        ,   dest_proc                   // target_rank
        ,   P::to_mpi_aint(dest_rptr)   // target_disp
        ,   static_cast<int>(num_bytes) // target_count
        ,   MPI_BYTE                    // target_datatype
        ,   win                         // win
        ,   request_result              // request
        });
    }
    
    void untyped_read_nb(
        const proc_id_type              src_proc
    ,   const remote_ptr<const void>&   src_rptr
    ,   const local_ptr<void>&          dest_lptr
    ,   const size_type                 num_bytes
    ,   request_type* const             request_result
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        const auto win = self.get_win();
        
        mi.rget({
            dest_lptr                   // origin_addr
        ,   static_cast<int>(num_bytes) // origin_count
        ,   MPI_BYTE                    // origin_datatype
        ,   src_proc                    // target_rank
        ,   P::to_mpi_aint(src_rptr)    // target_disp
        ,   static_cast<int>(num_bytes) // target_count
        ,   MPI_BYTE                    // target_datatype
        ,   win                         // win
        ,   request_result              // request
        });
    }
    
    template <typename T>
    void exchange_nb(
        const proc_id_type          target_proc
    ,   const remote_ptr<T>&        target_rptr
    ,   const local_ptr<const T>&   value_lptr
    ,   const local_ptr<T>&         result_lptr
    ,   request_type* const         request_result
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        const auto win = self.get_win();
        
        const auto datatype = P::template get_mpi_datatype<T>();
        
        mi.rget_accumulate({
            value_lptr                  // origin_addr
        ,   1                           // origin_count
        ,   datatype                    // origin_datatype
        ,   result_lptr                 // result_addr
        ,   1                           // result_count
        ,   datatype                    // result_datatype
        ,   target_proc                 // target_rank
        ,   P::to_mpi_aint(target_rptr) // target_disp
        ,   1                           // target_count
        ,   datatype                    // target_datatype
        ,   MPI_REPLACE                 // op
        ,   win                         // win
        ,   request_result              // request
        });
    }
    
    template <typename T>
    void compare_and_swap_nb(
        const proc_id_type          target_proc
    ,   const remote_ptr<T>&        target_rptr
    ,   const local_ptr<const T>&   expected_lptr
    ,   const local_ptr<const T>&   desired_lptr
    ,   const local_ptr<T>&         result_lptr
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        const auto win = self.get_win();
        
        const auto datatype = P::template get_mpi_datatype<T>();
        
        mi.compare_and_swap({
            desired_lptr
        ,   expected_lptr
        ,   result_lptr
        ,   datatype
        ,   target_proc
        ,   P::to_mpi_aint(target_rptr)
        ,   win
        });
    }
    
    template <typename T>
    T atomic_read(
        const proc_id_type          src_proc
    ,   const remote_ptr<const T>&  src_rptr
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        const auto win = self.get_win();
        
        const auto datatype = P::template get_mpi_datatype<T>();
        
        T result = T();
        
        mi.fetch_and_op({
            nullptr
        ,   &result
        ,   datatype
        ,   src_proc
        ,   P::to_mpi_aint(src_rptr)
        ,   MPI_NO_OP
        ,   win
        });
        
        this->flush_local(src_proc);
        
        return result;
    }
    
    template <typename T>
    void atomic_write(
        const proc_id_type          dest_proc
    ,   const remote_ptr<const T>&  dest_rptr
    ,   const T&                    value
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        const auto win = self.get_win();
        
        const auto datatype = P::template get_mpi_datatype<T>();
        
        T result = T();
        
        mi.fetch_and_op({
            &value
        ,   &result
        ,   datatype
        ,   dest_proc
        ,   P::to_mpi_aint(dest_rptr)
        ,   MPI_REPLACE
        ,   win
        });
        
        this->flush_local(dest_proc);
    }
    
    MEFDN_NODISCARD
    bool test(request_type* const req)
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        
        int flag;
        mi.test({ req, &flag, MPI_STATUS_IGNORE });
        
        return flag != 0;
    }
    
    void wait(request_type* const req)
    {
        #ifdef MECOM2_AVOID_MPI_WAIT
        while (!this->test(req))
        {
            #ifdef MEDEV2_AVOID_SWITCH_IN_SIGNAL
            if (! com_signal_state::is_in_signal()) {
                ult_itf_type::this_thread::yield();
            }
            #else
            ult_itf_type::this_thread::yield();
            #endif
        }
        #else
        
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        
        mi.wait({ req, MPI_STATUS_IGNORE });
        
        #endif
    }
    
    void flush()
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        const auto win = self.get_win();
        
        mi.win_flush_all({ win });
    }
    
    void flush(const proc_id_type proc)
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        const auto win = self.get_win();
        
        mi.win_flush({ proc, win });
    }
    
    void flush_local(const proc_id_type proc)
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        const auto win = self.get_win();
        
        mi.win_flush_local({ proc, win });
    }
};

} // namespace mecom2
} // namespace menps


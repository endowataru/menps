
#pragma once

#include <menps/mecom2/rma/rma_typed_handle.hpp>
#include <menps/medev2/mpi/mpi_datatype.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_mpi_rma_handle
    : public rma_typed_handle<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using process_id_type = typename P::process_id_type;
    using size_type       = typename P::size_type;
    
    using void_lptr_type    = typename P::template remote_ptr<void>;
    using void_rptr_type    = typename P::template local_ptr<void>;
    using cvoid_rptr_type   = typename P::template remote_ptr<const void>;
    using cvoid_lptr_type   = typename P::template local_ptr<const void>;
    
public:
    void untyped_read_nb(
        const process_id_type   src_proc
    ,   cvoid_rptr_type         src_rptr
    ,   void_lptr_type          dest_lptr
    ,   const size_type         size_in_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto win = self.get_win();
        
        MEFDN_LOG_VERBOSE(
            "msg:Do RMA read.\t"
            "src_proc:{}\t"
            "src_rptr:0x{:x}\t"
            "dest_lptr:0x{:x}\t"
            "size_in_bytes:{}"
        ,   src_proc
        ,   P::to_intptr(src_rptr)
        ,   P::to_intptr(dest_lptr)
        ,   size_in_bytes
        );
        
        mi.get({
            dest_lptr
        ,   src_proc
        ,   P::to_mpi_aint(src_rptr)
        ,   static_cast<int>(size_in_bytes)
        ,   win
        });
    }
    
    void untyped_write_nb(
        const process_id_type   dest_proc
    ,   void_rptr_type          dest_rptr
    ,   cvoid_lptr_type         src_lptr
    ,   const size_type         size_in_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto win = self.get_win();
        
        MEFDN_LOG_VERBOSE(
            "msg:Do RMA put.\t"
            "dest_proc:{}\t"
            "dest_rptr:0x{:x}\t"
            "src_lptr:0x{:x}\t"
            "size_in_bytes:{}"
        ,   dest_proc
        ,   P::to_intptr(dest_rptr)
        ,   P::to_intptr(src_lptr)
        ,   size_in_bytes
        );
        
        mi.put({
            src_lptr
        ,   dest_proc
        ,   P::to_mpi_aint(dest_rptr)
        ,   static_cast<int>(size_in_bytes)
        ,   win
        });
    }
    
    template <typename TargetPtr, typename ExpectedPtr, typename DesiredPtr, typename ResultPtr>
    void compare_and_swap_nb(
        const process_id_type   target_proc
    ,   TargetPtr&&             target_rptr
    ,   ExpectedPtr&&           expected_ptr
    ,   DesiredPtr&&            desired_ptr
    ,   ResultPtr&&             result_ptr
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto win = self.get_win();
        
        self.template check_cas_type_safety<TargetPtr, ExpectedPtr, DesiredPtr, ResultPtr>();
        
        using target_ptr_type   = mefdn::decay_t<TargetPtr>;
        using elem_type         = typename P::template element_type_of<target_ptr_type>;
        
        const auto datatype = medev2::mpi::get_datatype<elem_type>{}();
        
        MEFDN_LOG_VERBOSE(
            "msg:Do remote CAS.\t"
            "target_proc:{}\t"
            "target_rptr:0x{:x}\t"
            "desired_ptr:0x{:x}\t"
            "expected_ptr:0x{:x}\t"
            "result_ptr:0x{:x}"
        ,   target_proc
        ,   P::to_intptr(target_rptr)
        ,   P::to_intptr(desired_ptr)
        ,   P::to_intptr(expected_ptr)
        ,   P::to_intptr(result_ptr)
        );
        
        mi.compare_and_swap({
            desired_ptr
        ,   expected_ptr
        ,   result_ptr
        ,   datatype
        ,   target_proc
        ,   P::to_mpi_aint(target_rptr)
        ,   win
        });
    }
    
    void flush()
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto win = self.get_win();
        
        MEFDN_LOG_VERBOSE(
            "msg:Flush RMA requests."
        );
        
        mi.win_flush_all({ win });
    }
};

} // namespace mecom2
} // namespace menps


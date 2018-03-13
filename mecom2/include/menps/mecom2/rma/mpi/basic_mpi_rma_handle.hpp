
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
        
        mi.get({
            dest_lptr
        ,   src_proc
        ,   P::to_mpi_aint(src_rptr)
        ,   size_in_bytes
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
        
        mi.put({
            src_lptr
        ,   dest_proc
        ,   P::to_mpi_aint(dest_rptr)
        ,   size_in_bytes
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
        
        MEFDN_LOG_WARN(
            "msg:Do remote CAS.\t"
            "target_rptr:{:x}\t"
            "desired_ptr:{:x}\t"
            "expected_ptr:{:x}\t"
            "result_ptr:{:x}"
        ,   reinterpret_cast<mefdn::uintptr_t>(target_rptr)
        ,   reinterpret_cast<mefdn::uintptr_t>(desired_ptr)
        ,   reinterpret_cast<mefdn::uintptr_t>(expected_ptr)
        ,   reinterpret_cast<mefdn::uintptr_t>(result_ptr)
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
        
        mi.win_flush_all({ win });
    }
};

} // namespace mecom2
} // namespace menps



#pragma once

#include <menps/mecom2/rma/rma_typed_handle.hpp>
#include <menps/medev2/mpi/mpi.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_mpi_rma_handle
    : public rma_typed<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using process_id_type = typename P::process_id_type;
    using size_type       = typename P::size_type;
    
    using remote_ptr_type = typename P::template remote_ptr<void>;
    using local_ptr_type  = typename P::template local_ptr<void>;
    
public:
    void untyped_read_nb(
        const process_id_type   src_proc
    ,   remote_ptr_type         src_rptr
    ,   local_ptr_type          dest_lptr
    ,   const size_type         size_in_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto win = self.get_win();
        
        mi.get({
            dest_lptr
        ,   src_proc
        ,   reinterpret_cast<MPI_Aint>(src_rptr)
        ,   size_in_bytes
        ,   win
        });
    }
    
    void untyped_write_nb(
        const process_id_type   dest_proc
    ,   remote_ptr_type         dest_rptr
    ,   local_ptr_type          src_lptr
    ,   const size_type         size_in_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto win = self.get_win();
        
        mi.put({
            src_lptr
        ,   dest_proc
        ,   reinterpret_cast<MPI_Aint>(dest_rptr)
        ,   size_in_bytes
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


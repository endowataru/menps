
#pragma once

#include <menps/mecom2/coll/coll_typed.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_mpi_coll
    : public coll_typed<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using base = coll_typed<P>;
    
public:
    using typename base::proc_id_type;
    using typename base::size_type;
    
    void barrier()
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto comm = self.get_communicator();
        
        MEFDN_LOG_VERBOSE("msg:Call MPI_barrier().");
        
        mi.barrier({ comm });
    }
    
    void untyped_broadcast(
        const proc_id_type  root_proc
    ,   void* const         ptr
    ,   const size_type     num_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto comm = self.get_communicator();
        
        MEFDN_LOG_VERBOSE(
            "msg:Call MPI_Bcast().\t"
            "root_proc:{}\t"
            "ptr:0x{:x}\t"
            "num_bytes:{}\t"
        ,   root_proc
        ,   reinterpret_cast<mefdn::intptr_t>(ptr)
        ,   num_bytes
        );
        
        mi.broadcast({ ptr, num_bytes, root_proc, comm });
    }
    
    void untyped_allgather(
        const void* const   src_ptr
    ,   void* const         dest_ptr
    ,   const size_type     num_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto comm = self.get_communicator();
        const auto num_bytes_int = static_cast<int>(num_bytes);
        
        MEFDN_LOG_VERBOSE(
            "msg:Call MPI_Allgather().\t"
            "src_ptr:0x{:x}\t"
            "dest_ptr:0x{:x}\t"
            "num_bytes:{}\t"
        ,   reinterpret_cast<mefdn::intptr_t>(src_ptr)
        ,   reinterpret_cast<mefdn::intptr_t>(dest_ptr)
        ,   num_bytes
        );
        
        mi.allgather({ src_ptr, dest_ptr, num_bytes_int, comm });
    }
    
    void untyped_alltoall(
        const void* const   src_ptr
    ,   void* const         dest_ptr
    ,   const size_type     num_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        const auto comm = self.get_communicator();
        
        MEFDN_LOG_VERBOSE(
            "msg:Call MPI_Alltoall().\t"
            "src_ptr:0x{:x}\t"
            "dest_ptr:0x{:x}\t"
            "num_bytes:{}\t"
        ,   reinterpret_cast<mefdn::intptr_t>(src_ptr)
        ,   reinterpret_cast<mefdn::intptr_t>(dest_ptr)
        ,   num_bytes
        );
        
        mi.alltoall({ src_ptr, dest_ptr, num_bytes, comm });
    }
};

} // namespace mecom2
} // namespace menps


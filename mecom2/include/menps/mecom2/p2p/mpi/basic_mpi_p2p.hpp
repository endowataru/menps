
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_mpi_p2p
{
    MEFDN_DEFINE_DERIVED(P)
    
    using proc_id_type = typename P::proc_id_type;
    using size_type  = typename P::size_type;
    using tag_type  = typename P::tag_type;
    
public:
    void untyped_send(
        const proc_id_type  proc
    ,   const tag_type      tag
    ,   const void* const   buf
    ,   const size_type     num_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_itf();
        
        const auto comm = self.get_comm();
        
        mi.send({
            buf
        ,   num_bytes
        ,   proc
        ,   tag
        ,   comm
        });
    }
    
    void untyped_recv(
        const proc_id_type  proc
    ,   const tag_type      tag
    ,   void* const         buf
    ,   const size_type     num_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_itf();
        
        const auto comm = self.get_comm();
        
        mi.recv({
            buf
        ,   num_bytes
        ,   proc
        ,   tag
        ,   comm
        ,   MPI_STATUS_IGNORE
            // TODO: How to deal with status?
        });
    }
    
    template <typename T>
    void send(
        const proc_id_type  proc
    ,   const tag_type      tag
    ,   const T* const      buf
    ,   const size_type     num_elems
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_itf();
        
        const auto comm = self.get_comm();
        
        mi.send({
            buf
        ,   num_elems * sizeof(T)
        ,   proc
        ,   tag
        ,   comm
        });
    }
    
    template <typename T>
    void recv(
        const proc_id_type  proc
    ,   const tag_type      tag
    ,   T* const            buf
    ,   const size_type     num_elems
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_itf();
        
        const auto comm = self.get_comm();
        
        mi.recv({
            buf
        ,   num_elems * sizeof(T)
        ,   proc
        ,   tag
        ,   comm
        ,   MPI_STATUS_IGNORE
            // TODO: How to deal with status?
        });
    }
};

} // namespace mecom2
} // namespace menps


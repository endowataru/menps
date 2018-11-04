
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/logger.hpp>
#ifdef MEDEV2_AVOID_SWITCH_IN_SIGNAL
    #include <menps/mecom2/com/com_signal_state.hpp>
#endif

//#define MECOM2_USE_BLOCKING_MPI_P2P

namespace menps {
namespace mecom2 {

template <typename P>
class basic_mpi_p2p
{
    MEFDN_DEFINE_DERIVED(P)
    
    using proc_id_type = typename P::proc_id_type;
    using size_type  = typename P::size_type;
    using tag_type  = typename P::tag_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    
public:
    void untyped_send(
        const proc_id_type  proc
    ,   const tag_type      tag
    ,   const void* const   buf
    ,   const size_type     num_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        
        const auto comm = self.get_comm();
        
        #ifdef MECOM2_USE_BLOCKING_MPI_P2P
        
        mi.send({
            buf
        ,   num_bytes
        ,   proc
        ,   tag
        ,   comm
        });
        
        #else
        
        MPI_Request req;
        
        mi.isend({
            buf
        ,   static_cast<int>(num_bytes)
        ,   MPI_BYTE
        ,   proc
        ,   tag
        ,   comm
        ,   &req
        });
        
        this->wait(&req);
        
        #endif
    }
    
    void untyped_recv(
        const proc_id_type  proc
    ,   const tag_type      tag
    ,   void* const         buf
    ,   const size_type     num_bytes
    ) {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        
        const auto comm = self.get_comm();
        
        #ifdef MECOM2_USE_BLOCKING_MPI_P2P
        
        mi.recv({
            buf
        ,   num_bytes
        ,   proc
        ,   tag
        ,   comm
        ,   MPI_STATUS_IGNORE
            // TODO: How to deal with status?
        });
        
        #else
        
        MPI_Request req;
        
        mi.irecv({
            buf
        ,   static_cast<int>(num_bytes)
        ,   MPI_BYTE
        ,   proc
        ,   tag
        ,   comm
        ,   &req
        });
        
        this->wait(&req);
        
        #endif
    }
    
    template <typename T>
    void send(
        const proc_id_type  proc
    ,   const tag_type      tag
    ,   const T* const      buf
    ,   const size_type     num_elems
    ) {
        this->untyped_send(proc, tag, buf,
            num_elems * sizeof(T));
    }
    
    template <typename T>
    void recv(
        const proc_id_type  proc
    ,   const tag_type      tag
    ,   T* const            buf
    ,   const size_type     num_elems
    ) {
        this->untyped_recv(proc, tag, buf,
            num_elems * sizeof(T));
    }
    
private:
    void wait(MPI_Request* const req)
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_facade();
        
        #ifdef MECOM2_AVOID_MPI_WAIT
        while (true) {
            int flag;
            mi.test({ req, &flag, MPI_STATUS_IGNORE });
            // TODO: How to deal with status?
            
            if (flag != 0) { break; }
            
            #ifdef MEDEV2_AVOID_SWITCH_IN_SIGNAL
            if (! com_signal_state::is_in_signal()) {
                ult_itf_type::this_thread::yield();
            }
            #else
            ult_itf_type::this_thread::yield();
            #endif
        }
        
        #else
        mi.wait({ req, MPI_STATUS_IGNORE });
        
        #endif
    }
};

} // namespace mecom2
} // namespace menps

